#include "./RSMConfig.hpp"
#include "./RSMRules.hpp"
#include <zec/String.hpp>

#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

ZEC_NS
{
    static const char ResponseOk[] = "{ \"status\": 200 }";
    static void SendHttpOk(evhttp_request * Request) {
        auto OutputHeaders = evhttp_request_get_output_headers(Request);
        evhttp_add_header(OutputHeaders, "Content-Type", "application/json");
        evhttp_add_header(OutputHeaders, "Connection", "close");
        evbuffer * ResponseBuffer = evbuffer_new();
        evbuffer_add(ResponseBuffer, ResponseOk, SafeLength(ResponseOk));
        evhttp_send_reply(Request, HTTP_OK, "OK", ResponseBuffer);
        evbuffer_free(ResponseBuffer);
    }

    void RsmHttpConfigCallback(evhttp_request * Request, void *ContextPtr)
    {
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

            const char * ExpireStr = evhttp_find_header(&Queries, "expire");
            xRsmAddr Addr;
            if (!Addr.From(ProxyIpStr, ProxyPort)) {
                evhttp_send_error(Request, HTTP_INTERNAL, "ProxyAddressError");
                return;
            }

            auto SockAddrPtr = evhttp_connection_get_addr(evhttp_request_get_connection(Request));
            xRsmRule Rule = {
                { Addr },
                { .Timeout = ExpireStr ? (uint64_t)atol(ExpireStr) : RSM_GetConfig().ProxyExpire }
            };

            const char * ProxyUser = evhttp_find_header(&Queries, "user");
            const char * ProxyPass = evhttp_find_header(&Queries, "pass");
            if (ProxyUser && ProxyUser[0] && ProxyPass && ProxyPass[0]) {
                Rule.Sock5Proxy.Auth = xRsmProxyAuth{ ProxyUser, ProxyPass };
            }

            if (!RSM_SetProxyRule(std::move(Rule), SockAddrPtr)) {
                evhttp_send_error(Request, HTTP_BADREQUEST, "Blacklist");
                return;
            }
            RSM_LogI("UpdateProxyRule, Source=%s, Proxy=%s", ToString(SockAddrPtr).c_str(), Addr.ToString().c_str());
            SendHttpOk(Request);
            return;
        }

        else if (0 == strcmp(Path, "/op_auto_rm_proxy")) {
            RSM_UnsetProxyRule(evhttp_connection_get_addr(evhttp_request_get_connection(Request)));
            SendHttpOk(Request);
            return;
        }

        evhttp_send_error(Request, HTTP_NOTIMPLEMENTED, "InvalidOperation");
    }
}
