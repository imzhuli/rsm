#pragma once
#include "./Base/RSMBase.hpp"
#include <redsocks_multi/RSM.hpp>

ZEC_NS
{

    ZEC_PRIVATE event_base * _EventBase;
    ZEC_INLINE  event_base * RSM_GetBase() { return _EventBase; }

}
