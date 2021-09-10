#pragma once
#include <redsocks_multi/RSMConfig.hpp>
#include <event2/http.h>

ZEC_NS
{

    ZEC_PRIVATE void HttpConfigCallback(evhttp_request * Request, void * ContextPtr);

}