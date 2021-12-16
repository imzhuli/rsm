#pragma once
#include <zec/Common.hpp>
#include <zec/Util/Logger.hpp>
#include "./RSMConfig.hpp"

ZEC_NS
{

    ZEC_API bool RSM_Init(const xRsmConfig & Config = {}, xLogger * LoggerPtr = nullptr);
    ZEC_API bool RSM_IsReady();
    ZEC_API void RSM_Run();
    ZEC_API void RSM_Stop();
    ZEC_API void RSM_Clean();
    ZEC_API const xRsmConfig & RSM_GetConfig();

    ZEC_API xLogger * RSM_GetLogger();

    template<typename ... tArgs>
    void RSM_LogD(const char * Fmt, tArgs && ... Args) {
        RSM_GetLogger()->D(Fmt, std::forward<tArgs>(Args)...);
    }

    template<typename ... tArgs>
    void RSM_LogI(const char * Fmt, tArgs && ... Args) {
        RSM_GetLogger()->I(Fmt, std::forward<tArgs>(Args)...);
    }

    template<typename ... tArgs>
    void RSM_LogE(const char * Fmt, tArgs && ... Args) {
        RSM_GetLogger()->E(Fmt, std::forward<tArgs>(Args)...);
    }

}
