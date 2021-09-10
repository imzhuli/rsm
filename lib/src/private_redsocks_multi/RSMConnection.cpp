#include "./RSMConnection.hpp"

ZEC_NS
{

    void ProxyEntryCallback(evconnlistener * Listener, evutil_socket_t SocketFd, sockaddr * Addr, int AddrLen, void * ContextPtr)
    {
        evutil_closesocket(SocketFd);
    }

}
