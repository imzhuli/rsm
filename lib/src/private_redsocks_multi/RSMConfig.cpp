#include "./RSMConfig.hpp"

#include <event2/http.h>
#include <event2/buffer.h>

ZEC_NS
{

    void HttpConfigCallback(evhttp_request * Request, void * ContextPtr)
    {
        struct evbuffer *Buffer = evbuffer_new();
        if (!Buffer)
        {
            puts("failed to create response buffer \n");
            return;
        }
        evbuffer_add_printf(Buffer, "Server Responsed. Requested: %s\n", evhttp_request_get_uri(Request));
        evhttp_send_reply(Request, HTTP_OK, "OK", Buffer);
        evbuffer_free(Buffer);
    }

}
