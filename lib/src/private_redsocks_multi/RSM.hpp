#pragma once
#include "./Base/RSMBase.hpp"

ZEC_NS
{

    ZEC_PRIVATE event_base * _EventBase;
    ZEC_INLINE  event_base * RSM_GetBase() { return _EventBase; }

}
