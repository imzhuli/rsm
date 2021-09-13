#include "./RSMConnection.hpp"
#include "./RSMRules.hpp"
#include "./RSM.hpp"
#include "./NetFilter/IpTables.hpp"
#include <zec/String.hpp>

ZEC_NS
{

    static xRsmTimeoutList S5ProxyConnectionTimeoutList;
    static xRsmDeleteList S5ProxyConnectionDeleteList;

    void xRsmProxyConnectionOperator::ServerEntryCallback
        (evconnlistener * Listener, evutil_socket_t SocketFd, sockaddr * SourceAddr, int AddrLen, void *ContextPtr)
    {
        ConnectionsPerMinute.fetch_add(1);
        xRsmAddr RsmTargetAddr;
        if (!Rsm_GetOriginalTarget(SocketFd, xRef(RsmTargetAddr))) {
            RSM_LogE("Failed to GetOriginalTargetAddr");
            evutil_closesocket(SocketFd);
            return;
        }
        auto RsmRulePtr = RSM_GetProxyRule(SourceAddr, &RsmTargetAddr.SockAddr);
        if (!RsmRulePtr) {
            RSM_LogE("NoProxyRule Found!");
            evutil_closesocket(SocketFd);
            return;
        }
        // setup 2-way connection:
        auto ConnectionPtr = new xRsmProxyConnection();
        if (!ConnectionPtr->Connect(RSM_GetBase(), Steal(SocketFd, -1), RsmRulePtr->Sock5Proxy, RsmTargetAddr)) {
            delete ConnectionPtr;
            return;
        }
    }

    void xRsmProxyConnectionOperator::ClientReadCallback(struct bufferevent * bev, void * ContextPtr)
    {
        auto ProxyConnectionPtr = (xRsmProxyConnection *)ContextPtr;
        if (!ProxyConnectionPtr->ProxyDelegate.OnClientData()) {
            ProxyConnectionPtr->OnProxyDisconnected();
        }
    }

    void xRsmProxyConnectionOperator::ClientEventCallback(struct bufferevent * bev, short events, void * ContextPtr)
    {
        auto ProxyConnectionPtr = (xRsmProxyConnection *)ContextPtr;
        if (events & BEV_EVENT_CONNECTED) {
            RSM_LogD("Unexpected event on client connection");
            return;
        }
        RSM_LogD("Client disconnected: %p, Event=%x", ContextPtr, (int)events);
        ProxyConnectionPtr->OnClientDisconnected();
    }

    void xRsmProxyConnectionOperator::ProxyReadCallback(struct bufferevent * bev, void * ContextPtr)
    {
        auto ProxyConnectionPtr = (xRsmProxyConnection *)ContextPtr;
        if (!ProxyConnectionPtr->ProxyDelegate.OnProxyData()) {
            ProxyConnectionPtr->OnProxyDisconnected();
        }
    }

    void xRsmProxyConnectionOperator::ProxyEventCallback(struct bufferevent * bev, short events, void * ContextPtr)
    {
        auto ProxyConnectionPtr = (xRsmProxyConnection *)ContextPtr;
        if (events & BEV_EVENT_CONNECTED) {
            RSM_LogD("Proxy connected: %p", ContextPtr);
            ProxyConnectionPtr->OnProxyConnected();
        }
        else {
            RSM_LogD("Proxy connection peer closed: %p, Event=%x", ContextPtr, (int)events);
            ProxyConnectionPtr->OnProxyDisconnected();
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

    bool xRsmProxyConnection::Connect(event_base * EventBasePtr, evutil_socket_t && ClientSocketFd, const xRsmS5Proxy& Proxy, const xRsmAddr &TargetAddr)
    {
        assert(EventBasePtr);
        do {
            if (!(ClientEventPtr = bufferevent_socket_new(EventBasePtr, ClientSocketFd, BEV_OPT_DEFER_CALLBACKS | BEV_OPT_CLOSE_ON_FREE))) {
                evutil_closesocket(ClientSocketFd);
                goto LABEL_ERROR;
            }
            bufferevent_setcb(ClientEventPtr,
                xRsmProxyConnectionOperator::ClientReadCallback,
                nullptr,
                xRsmProxyConnectionOperator::ClientEventCallback,
                this);
            bufferevent_enable(ClientEventPtr, EV_READ);

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
            if (bufferevent_enable(ProxyEventPtr, EV_READ)
                 || bufferevent_socket_connect(ProxyEventPtr, Proxy.Addr.GetSockAddr(), (int)Proxy.Addr.GetSockAddrLen())
            ) {
                goto LABEL_ERROR;
            }
        } while(false);

        do {
            if (!ProxyDelegate.Init(ClientEventPtr, ProxyEventPtr, TargetAddr, Proxy.Auth)) {
                goto LABEL_ERROR;
            }

        } while(false);

        ++TotalConnections;
        IsOpen = true;
        return true;

    LABEL_ERROR:
        if (ProxyEventPtr) {
            bufferevent_free(Steal(ProxyEventPtr));
        }
        if (ClientEventPtr) {
            bufferevent_free(Steal(ClientEventPtr));
        }
        return false;
    }

    void xRsmProxyConnection::Disconnect()
    {
        if (!Steal(IsOpen)) {
            return;
        }
        ProxyDelegate.Clean();
        bufferevent_free(Steal(ProxyEventPtr));
        bufferevent_free(Steal(ClientEventPtr));
        --TotalConnections;
    }

    xRsmProxyConnection::~xRsmProxyConnection()
    {
        Disconnect();
    }

    void xRsmProxyConnection::OnProxyConnected()
    {
        if (!ProxyDelegate.OnProxyConnected()) {
            Disconnect();
            xRsmProxyConnectionOperator::DeferDelete(this);
        }
    }

    void xRsmProxyConnection::OnProxyDisconnected()
    {
        Disconnect();
        xRsmProxyConnectionOperator::DeferDelete(this);
    }

    void xRsmProxyConnection::OnClientDisconnected()
    {
        Disconnect();
        xRsmProxyConnectionOperator::DeferDelete(this);
    }

}
