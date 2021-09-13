#pragma once
#include "./Base/RSMBase.hpp"
#include "./RSMS5.hpp"
#include "./RSMRules.hpp"

ZEC_NS
{
    class xRsmProxyConnection;
    class xRsmProxyConnectionOperator;

    class xRsmProxyConnectionOperator final
    {
    public:
        ZEC_PRIVATE_MEMBER static void ServerEntryCallback
            (evconnlistener * Listener, evutil_socket_t SocketFd, sockaddr * Addr, int AddrLen, void * ContextPtr);

        ZEC_PRIVATE_MEMBER static void ClientReadCallback(struct bufferevent * bev, void * ContextPtr);
        ZEC_PRIVATE_MEMBER static void ClientEventCallback(struct bufferevent * bev, short events, void * ContextPtr);

        ZEC_PRIVATE_MEMBER static void ProxyReadCallback(struct bufferevent * bev, void * ContextPtr);
        ZEC_PRIVATE_MEMBER static void ProxyEventCallback(struct bufferevent * bev, short events, void * ContextPtr);

        ZEC_PRIVATE_MEMBER static void DeferDelete(xRsmProxyConnection * ConnectionPtr);
        ZEC_PRIVATE_MEMBER static void CommitDeferedDeletions();
    };

    class xRsmProxyConnection final
    : public xRsmTimeoutNode
    , public xRsmDeleteNode
    , xNonCopyable
    {
    public:
        ZEC_PRIVATE_MEMBER bool Connect(event_base * EventBasePtr, evutil_socket_t && ClientSocketFd,
            const xRsmS5Proxy& Rule, const xRsmAddr &TargetAddr);
        ZEC_PRIVATE_MEMBER void Disconnect();

    private:
        friend class xRsmProxyConnectionOperator;

        ZEC_PRIVATE_MEMBER ~xRsmProxyConnection();
        ZEC_PRIVATE_MEMBER void OnProxyConnected();
        ZEC_PRIVATE_MEMBER void OnProxyDisconnected();
        ZEC_PRIVATE_MEMBER void OnClientDisconnected();

        bufferevent * ClientEventPtr = nullptr;
        bufferevent * ProxyEventPtr = nullptr;

        xS5ProxyDelegate ProxyDelegate;
        bool IsOpen = false;
    };

}

