#pragma once
#include "./Base/RSMBase.hpp"
#include <event2/http.h>

ZEC_NS
{

    ZEC_PRIVATE void RsmHttpConfigCallback(evhttp_request * Request, void * ContextPtr);

}