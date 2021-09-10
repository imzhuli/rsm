#pragma once
#include <zec/Common.hpp>

#include <event2/event.h>
#include <event2/util.h>
#include <event2/listener.h>

ZEC_NS
{

    struct xRsmConnection
    {
        enum eState {
            Inited = 0,
            Accepted,
            ConnectingToProxy,
            WaitingAuthorization,
            AuthorizationDone,
            AuthorizationRejected,
            Closed,
        };

        
        eState State; 

    };

    ZEC_PRIVATE void ProxyEntryCallback(evconnlistener * Listener, evutil_socket_t SocketFd, sockaddr * Addr, int AddrLen, void * ContextPtr);

}

