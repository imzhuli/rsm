#include <redsocks_multi/RSM.hpp>
#include <atomic>
#include <mutex>

#include <event2/event.h>
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

    static struct event_base * _EventBase = nullptr;

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

        do {
            if (evthread_use_pthreads()) {
                return false;
            }
            event_set_fatal_callback(&OnLibeventFatalError);

            if (!(_EventBase = event_base_new())) {
                libevent_global_shutdown();
                return false;
            }
        } while(false);
        
        _Config = Config;
        _LoggerPtr = LoggerPtr;
        return _Inited = true;
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
        event_base_free(Steal(_EventBase));
        libevent_global_shutdown();
        Reset(_Config);
        _LoggerPtr = NonLoggerPtr;
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
