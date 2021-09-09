#include <redsocks_multi/RSM.hpp>
#include <atomic>
#include <mutex>

#include <event2/event.h>
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
        event_set_fatal_callback(&OnLibeventFatalError);
        if (evthread_use_pthreads()) {
            goto LABEL_ERROR;
        }
        if (!(_EventBase = event_base_new())) {
            goto LABEL_ERROR;
        }
        if (!(_HttpConfigListener = evhttp_new(_EventBase))) {
            goto LABEL_ERROR;
        }
        if (evhttp_bind_socket(_HttpConfigListener, _Config.ConfigIp.c_str(), _Config.ConfigPort)) {
            goto LABEL_ERROR;
        }
        
        _LoggerPtr = LoggerPtr;
        return _Inited = true;

    LABEL_ERROR:
        if (_HttpConfigListener) {
            evhttp_free(Steal(_HttpConfigListener));
        }
        if (_EventBase) {
            event_base_free(Steal(_EventBase));
        }
        libevent_global_shutdown();
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

        _LoggerPtr = NonLoggerPtr;
        evhttp_free(Steal(_HttpConfigListener));
        event_base_free(Steal(_EventBase));
        libevent_global_shutdown();
        Reset(_Config);
        _Inited = false;
    }
    
    xLogger * RSM_GetLogger()
    {
        return _LoggerPtr;
    }

    void RSM_Run()
    {

    }

}
