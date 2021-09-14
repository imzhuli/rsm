#include "./RSMConfig.hpp"
#include "./RSMRules.hpp"
#include "./RSM.hpp"
#include "./RSMConnection.hpp"

#include <atomic>
#include <mutex>

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/http.h>
#include <event2/thread.h>

#ifndef EVTHREAD_USE_PTHREADS_IMPLEMENTED
#error "Requires EVTHREAD_USE_PTHREADS_IMPLEMENTED"
#endif

ZEC_NS
{
    static std::mutex          _InitMutex;
    static std::atomic_bool    _Inited = false;

    static xRsmConfig          _Config;
    static xLogger *           _LoggerPtr = NonLoggerPtr;

    static std::atomic_bool    _KeepRunning = false;
    static evhttp *            _HttpConfigListener = nullptr;
    static evconnlistener *    _ConnectionListener = nullptr;
    static event *             _EventTicker = nullptr;

    event_base * _EventBase = nullptr;

    std::atomic_uint64_t TotalExactTargetRules = 0;
    std::atomic_uint64_t TotalExactSourceRules = 0;
    std::atomic_uint64_t TotalIpOnlyTargetRules = 0;
    std::atomic_uint64_t TotalIpOnlySourceRules = 0;

    std::atomic_uint64_t PeriodConnections = 0;
    std::atomic_uint64_t PeriodDataIn = 0;
    std::atomic_uint64_t PeriodDataOut = 0;
    std::atomic_uint64_t TotalConnections;

    static void OnLibeventFatalError(int err)
    {
        RSM_LogE("Libevent Fatal Error: ErrCode=%i", err);
    }

    const xRsmConfig & RSM_GetConfig()
    {
        return _Config;
    }

    static void OnTimerTick(int /*sock*/, short /*event*/, void * ContextPtr)
    {
        event_base_loopbreak(static_cast<event_base *>(ContextPtr));
    }

    bool RSM_Init(const xRsmConfig & Config, xLogger * LoggerPtr)
    {
        auto Guard = std::lock_guard(_InitMutex);
        if (_Inited) {
            Error("Multiple Init");
            return false;
        }

        _Config = Config;
        _LoggerPtr = LoggerPtr;
        // setup libevent
        do {
            event_set_fatal_callback(&OnLibeventFatalError);
            if (evthread_use_pthreads()) {
                goto LABEL_ERROR;
            }
            if (!(_EventBase = event_base_new())) {
                goto LABEL_ERROR;
            }
            if (!(_EventTicker = event_new(_EventBase, -1, EV_PERSIST, OnTimerTick, _EventBase))) {
                goto LABEL_ERROR;
            }
            if (evtimer_add(_EventTicker, X2Ptr(timeval{ 0, 250'000 }))) {
                goto LABEL_ERROR;
            }
        } while(false);

        // config http server
        do {
            if (!(_HttpConfigListener = evhttp_new(_EventBase))) {
                goto LABEL_ERROR;
            }
            if (evhttp_bind_socket(_HttpConfigListener, _Config.ConfigIp.c_str(), _Config.ConfigPort)) {
                goto LABEL_ERROR;
            }
            evhttp_set_timeout(_HttpConfigListener, 1);
            evhttp_set_gencb(_HttpConfigListener, RsmHttpConfigCallback, nullptr);
        } while(false);

        // connection listener
        do {
            xRsmAddr Addr {};
            if (1 == evutil_inet_pton(AF_INET, _Config.EntryIp.c_str(), &Addr.Ipv4.sin_addr)) {
                Addr.Ipv4.sin_family = AF_INET;
                Addr.Ipv4.sin_port = htons(_Config.EntryPort);
                _ConnectionListener = evconnlistener_new_bind(_EventBase, xRsmProxyConnectionOperator::ServerEntryCallback, nullptr,
                    0, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                    Addr.GetSockAddr(), sizeof(Addr.Ipv4));
            } else if (1 == evutil_inet_pton(AF_INET6, _Config.EntryIp.c_str(), &Addr.Ipv6.sin6_addr)) {
                Addr.Ipv6.sin6_family = AF_INET6;
                Addr.Ipv6.sin6_port = htons(_Config.EntryPort);
                _ConnectionListener = evconnlistener_new_bind(_EventBase, xRsmProxyConnectionOperator::ServerEntryCallback, nullptr,
                    0, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                    Addr.GetSockAddr(), sizeof(Addr.Ipv6));
            } else {
                goto LABEL_ERROR;
            }
            if (!_ConnectionListener) {
                _LoggerPtr->E("Failed to create ProxyEntry");
                goto LABEL_ERROR;
            }

        } while(false);

        do {
            xRsmAddr ConfigAddr, EntryAddr;
            ConfigAddr.From(_Config.ConfigIp.c_str(), _Config.ConfigPort);
            EntryAddr.From(_Config.EntryIp.c_str(), _Config.EntryPort);
            RSM_SetProxyBlacklist(ConfigAddr.GetSockAddr());
            RSM_SetProxyBlacklist(EntryAddr.GetSockAddr());
        } while(false);
        return _Inited = true;

    LABEL_ERROR:
        if (_ConnectionListener) {
            evconnlistener_free(Steal(_ConnectionListener));
        }
        if (_HttpConfigListener) {
            evhttp_free(Steal(_HttpConfigListener));
        }
        if (_EventTicker) {
            event_free(Steal(_EventTicker));
        }
        if (_EventBase) {
            event_base_free(Steal(_EventBase));
        }
        libevent_global_shutdown();
        _LoggerPtr = NonLoggerPtr;
        Reset(_Config);
        return false;
    }

    bool RSM_IsReady()
    {
        return _Inited;
    }

    void RSM_Clean() {
        auto Guard = std::lock_guard(_InitMutex);
        if (!_Inited) {
            Error("RSM Not Inited");
        }

        RSM_ClearBlacklist();
        evconnlistener_free(Steal(_ConnectionListener));
        evhttp_free(Steal(_HttpConfigListener));
        event_free(Steal(_EventTicker));
        event_base_free(Steal(_EventBase));
        libevent_global_shutdown();
        _LoggerPtr = NonLoggerPtr;
        Reset(_Config);
        _Inited = false;
    }

    xLogger * RSM_GetLogger()
    {
        return _LoggerPtr;
    }

    xTimer StatisticTimer;
    static void RSM_LogStatistics()
    {
        if (!StatisticTimer.TestAndTag(60s)) {
            return;
        }
        RSM_LogI("StatisticTimeout: "
            "PeriodConnections=%u, "
            "TotalConnections=%u, "
            // "TotalExactTargetRules=%u, "
            // "TotalExactSourceRules=%u, "
            // "TotalIpOnlyTargetRules=%u, "
            "TotalIpOnlySourceRules=%u, "
            "DataInPerMinute=%" PRIu64 ", "
            "DataOutPerMinute=%" PRIu64 ", ",
            (unsigned int)PeriodConnections.exchange(0),
            (unsigned int)TotalConnections,
            // (unsigned int)TotalExactTargetRules,
            // (unsigned int)TotalExactSourceRules,
            // (unsigned int)TotalIpOnlyTargetRules,
            (unsigned int)TotalIpOnlySourceRules,
            PeriodDataIn.exchange(0),
            PeriodDataOut.exchange(0)
        );
    }

    void RSM_Run()
    {
        _KeepRunning = true;
        while(_KeepRunning) {
            event_base_loop(_EventBase, EVLOOP_ONCE);
            RSM_ClearTimeoutRules();
            RSM_LogStatistics();
        }
    }

    void RSM_Stop()
    {
        _KeepRunning = false;
    }

}
