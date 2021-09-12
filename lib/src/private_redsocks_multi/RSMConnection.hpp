#pragma once
#include "./Base/RSMBase.hpp"

ZEC_NS
{
    class xRsmProxyConnection;
    class xRsmProxyConnectionOperator;

    class xRsmProxyConnectionOperator final
    {
    public:
        static void ClientReadCallback(struct bufferevent * bev, void * ContextPtr);
        static void ClientEventCallback(struct bufferevent * bev, short events, void * ContextPtr);

        static void ProxyReadCallback(struct bufferevent * bev, void * ContextPtr);
        static void ProxyEventCallback(struct bufferevent * bev, short events, void * ContextPtr);

        static void DeferDelete(xRsmProxyConnection * ConnectionPtr);
        static void CommitDeferedDeletions();
    };

    class xRsmProxyConnection final
    : public xRsmTimeoutNode
    , public xRsmDeleteNode
    , xNonCopyable
    {
    public:
        enum struct eState {
            Inited = 0,
            Connecting,
            Authorization,
            Established,
            Closed,
        };
    
        bool Connect(event_base * EventBasePtr, evutil_socket_t ClientSocketFd, const xRsmAddr& S5ProxyAddr);
        void Disconnect();

    private:
        friend class xRsmProxyConnectionOperator;

        ~xRsmProxyConnection();
        void OnProxyConnected();
        void OnProxyDisconnected();

        eState State = eState::Inited;
        event_base * _EventBaseShadow = nullptr;
        
        bufferevent * ClientEventPtr = nullptr;
        bufferevent * ProxyEventPtr = nullptr;
    };

    ZEC_PRIVATE void ServerEntryCallback(evconnlistener * Listener, evutil_socket_t SocketFd, sockaddr * Addr, int AddrLen, void * ContextPtr);

}

