#include "./RSMConnection.hpp"
#include "./RSMRules.hpp"
#include "./NetFilter/IpTables.hpp"
#include <redsocks_multi/RSM.hpp>
#include <zec/String.hpp>

ZEC_NS
{

    static xRsmTimeoutList S5ProxyConnectionTimeoutList;
    static xRsmDeleteList S5ProxyConnectionDeleteList;

    void ServerEntryCallback(evconnlistener * Listener, evutil_socket_t SocketFd, sockaddr * SourceAddr, int AddrLen, void *ContextPtr)
    {
        ConnectionsPerMinute.fetch_add(1);
        xRsmAddr RsmAddr;
        if (GetOriginalTarget(SocketFd, xRef(RsmAddr))) {
            RSM_LogD("OriginConnectionTo: %s", StrToHex(RSM_MakeExactAddressKey(&RsmAddr.SockAddrBase)).c_str());
        }
        auto RsmRulePtr = RSM_GetProxyRule(SourceAddr, &RsmAddr.SockAddrBase);
        if (!RsmRulePtr) {
            RSM_LogI("NoProxyRule Found!");
            evutil_closesocket(SocketFd);
            return;
        }
    }

    void xRsmProxyConnectionOperator::ClientReadCallback(struct bufferevent * bev, void * ContextPtr)
    {

    }

    void xRsmProxyConnectionOperator::ClientEventCallback(struct bufferevent * bev, short events, void * ContextPtr)
    {
        auto ProxyConnectionPtr = (xRsmProxyConnection *)ContextPtr;
        if (events & BEV_EVENT_ERROR) {
            RSM_LogD("Client disconnected: %p", ContextPtr);
            ProxyConnectionPtr->Disconnect();
            ProxyConnectionPtr->OnProxyDisconnected();
            DeferDelete(ProxyConnectionPtr);
        } 
    }

    void xRsmProxyConnectionOperator::ProxyReadCallback(struct bufferevent * bev, void * ContextPtr)
    {
        // /* This callback is invoked when there is data to read on bev. */
        // struct evbuffer *input = bufferevent_get_input(bev);
        // struct evbuffer *output = bufferevent_get_output(bev);

        // ++total_messages_read;
        // total_bytes_read += evbuffer_get_length(input);

        // /* Copy all the data from the input buffer to the output buffer. */
        // evbuffer_add_buffer(output, input);
    }

    void xRsmProxyConnectionOperator::ProxyEventCallback(struct bufferevent * bev, short events, void * ContextPtr)
    {
        auto ProxyConnectionPtr = (xRsmProxyConnection *)ContextPtr;
        if (events & BEV_EVENT_ERROR) {
            RSM_LogD("Proxy disconnected: %p", ContextPtr);
            ProxyConnectionPtr->Disconnect();
            ProxyConnectionPtr->OnProxyDisconnected();
            DeferDelete(ProxyConnectionPtr);
        } 
        else if (events & BEV_EVENT_CONNECTED) {
            ProxyConnectionPtr->OnProxyConnected();
        }
    }
    
    void xRsmProxyConnectionOperator::DeferDelete(xRsmProxyConnection * ProxyConnectionPtr)
    {
        S5ProxyConnectionDeleteList.GrabTail(*ProxyConnectionPtr);
    }

    void xRsmProxyConnectionOperator::CommitDeferedDeletions()
    {
        for (auto & DeferedDeleteObject : S5ProxyConnectionDeleteList) {
            delete &DeferedDeleteObject;
        }
    }

    bool xRsmProxyConnection::Connect(event_base * EventBasePtr, evutil_socket_t ClientSocketFd, const xRsmAddr &S5ProxyAddr)
    {
        assert(State == eState::Inited);
        assert(EventBasePtr);
        do {
            if (!(ClientEventPtr = bufferevent_socket_new(EventBasePtr, ClientSocketFd, BEV_OPT_DEFER_CALLBACKS | BEV_OPT_CLOSE_ON_FREE))) {
                goto LABEL_ERROR;
            }
            bufferevent_setcb(ProxyEventPtr, 
                xRsmProxyConnectionOperator::ClientReadCallback, 
                nullptr, 
                xRsmProxyConnectionOperator::ClientEventCallback, 
                this);
            bufferevent_enable(ProxyEventPtr, EV_READ);

        } while(false);

        do {
            if (!(ProxyEventPtr = bufferevent_socket_new(EventBasePtr, -1, BEV_OPT_DEFER_CALLBACKS | BEV_OPT_CLOSE_ON_FREE))) {
                goto LABEL_ERROR;
            }
            setsockopt(bufferevent_getfd(ProxyEventPtr), IPPROTO_TCP, TCP_NODELAY,  X2Ptr((int)1), sizeof(int));
            bufferevent_setcb(ProxyEventPtr, 
                xRsmProxyConnectionOperator::ProxyReadCallback, 
                nullptr, 
                xRsmProxyConnectionOperator::ProxyEventCallback, 
                this);
            bufferevent_enable(ProxyEventPtr, EV_READ);
        } while(false);
        return true;

    LABEL_ERROR:
        if (ProxyEventPtr) {
            bufferevent_free(Steal(ProxyEventPtr));
        }
        if (ProxyEventPtr) {
            bufferevent_free(Steal(ClientEventPtr));
        }
        State = eState::Closed;
        return false;
    }

    void xRsmProxyConnection::Disconnect()
    {
        if (State == eState::Closed) {
            return;
        }
        assert(ProxyEventPtr);
        bufferevent_free(Steal(ProxyEventPtr));
        bufferevent_free(Steal(ClientEventPtr));
        State = eState::Closed;
    }

    xRsmProxyConnection::~xRsmProxyConnection()
    {
        Disconnect();
    }
   
    void xRsmProxyConnection::OnProxyConnected()
    {
    }
    
    void xRsmProxyConnection::OnProxyDisconnected()
    {
    }

}
