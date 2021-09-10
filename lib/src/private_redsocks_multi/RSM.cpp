#include "./RSMConfig.hpp"
#include "./RSMRules.hpp"
#include "./RSMConnection.hpp"

#include <redsocks_multi/RSM.hpp>
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

    static event_base * _EventBase = nullptr;
    static evhttp * _HttpConfigListener = nullptr;
    static evconnlistener * _ConnectionListener = nullptr;

    static void OnLibeventFatalError(int err)
    {
        RSM_LogE("Libevent Fatal Error: ErrCode=%i", err);
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
        } while(false);

        // config http server
        do {
            if (!(_HttpConfigListener = evhttp_new(_EventBase))) {
                goto LABEL_ERROR;
            }
            if (evhttp_bind_socket(_HttpConfigListener, _Config.ConfigIp.c_str(), _Config.ConfigPort)) {
                goto LABEL_ERROR;
            }
            evhttp_set_gencb(_HttpConfigListener, HttpConfigCallback, nullptr);
        } while(false);

        // connection listener
        do {
            xRsmAddr Addr {};
            if (1 == evutil_inet_pton(AF_INET, _Config.EntryIp.c_str(), &Addr.Ipv4)) {
                Addr.Ipv4.sin_family = AF_INET;
                Addr.Ipv4.sin_port = htons(_Config.EntryPort);
                _ConnectionListener = evconnlistener_new_bind(_EventBase, &ProxyEntryCallback, nullptr, 
                    0, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                    Addr.GetAddr(), sizeof(Addr.Ipv4));
            } else if (1 == evutil_inet_pton(AF_INET6, _Config.EntryIp.c_str(), &Addr.Ipv6)) {
                Addr.Ipv6.sin6_family = AF_INET6;
                Addr.Ipv6.sin6_port = htons(_Config.EntryPort);
                _ConnectionListener = evconnlistener_new_bind(_EventBase, &ProxyEntryCallback, nullptr, 
                    0, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                    Addr.GetAddr(), sizeof(Addr.Ipv6));
            } else {
                goto LABEL_ERROR;
            }
            if (!_ConnectionListener) {
                _LoggerPtr->E("Failed to create ProxyEntry");
                goto LABEL_ERROR;
            }

        } while(false);

        return _Inited = true;

    LABEL_ERROR:
        if (_ConnectionListener) {
            evconnlistener_free(Steal(_ConnectionListener));
        }
        if (_HttpConfigListener) {
            evhttp_free(Steal(_HttpConfigListener));
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

        evconnlistener_free(Steal(_ConnectionListener));
        evhttp_free(Steal(_HttpConfigListener));
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

    void RSM_Run()
    {
        event_base_dispatch(_EventBase);
    }

}
