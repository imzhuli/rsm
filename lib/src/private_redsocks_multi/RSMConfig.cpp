#include "./RSMConfig.hpp"
#include "./RSMRules.hpp"
#include <redsocks_multi/RSM.hpp>
#include <zec/String.hpp>

#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

ZEC_NS
{

    void HttpConfigCallback(evhttp_request * Request, void *ContextPtr)
    {
        auto OutputHeaders = evhttp_request_get_output_headers(Request);
        evhttp_add_header(OutputHeaders, "Connection", "close");

        auto Uri = evhttp_request_get_evhttp_uri(Request);
        auto Path = evhttp_uri_get_path(Uri);
        if (!Path) {
            evhttp_send_error(Request, HTTP_INTERNAL, "QueryPathError");
            return;
        }
        if (0 == strcmp(Path, "/op_auto_proxy")) {
            evkeyvalq Queries = {};
            auto Query = evhttp_uri_get_query(Uri);
            auto AutoCleanUp = xScopeGuard { [&]() {
                evhttp_clear_headers(&Queries);
            }};

            if (evhttp_parse_query_str(Query, &Queries)) {
                evhttp_send_error(Request, HTTP_BADREQUEST, "InvalidQueryString");
                return;
            }

            const char * ProxyIpStr = evhttp_find_header(&Queries, "ip");
            const char * ProxyPortStr = evhttp_find_header(&Queries, "port");
            if (!ProxyIpStr || !ProxyPortStr) {
                evhttp_send_error(Request, HTTP_BADREQUEST, "InvalidParams");
                return;
            }
            auto ProxyPort = (uint16_t)atol(ProxyPortStr);

            xRsmAddr Addr;
            if (!Addr.From(ProxyIpStr, ProxyPort)) {
                evhttp_send_error(Request, HTTP_INTERNAL, "ProxyAddressError");
                return;
            }

            auto SockAddrPtr = evhttp_connection_get_addr(evhttp_request_get_connection(Request));
            auto IpOnlySourceKey = RSM_MakeIpOnlyAddressKey(SockAddrPtr);
            RSM_LogD("ConfigSourceAddr: %s", StrToHex(IpOnlySourceKey).c_str());

            xRsmRule Rule = {
                { Addr },
                { .Timeout = 60 }
            };
            if (!RSM_SetProxyRule(std::move(Rule), SockAddrPtr)) {
                evhttp_send_error(Request, HTTP_BADREQUEST, "Blacklist");
                return;
            }
            evhttp_send_reply(Request, HTTP_OK, "Accepted", nullptr);
            return;
        }

        evhttp_send_error(Request, HTTP_NOTIMPLEMENTED, "InvalidOperation");
    }
}
