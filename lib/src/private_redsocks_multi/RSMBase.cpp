#include "./Base/RSMBase.hpp"
#include <cstring>

ZEC_NS
{

    static_assert(sizeof(sockaddr_storage) == sizeof(xRsmAddr));

    bool RSM_TestEqual(const sockaddr * Addr1, const sockaddr * Addr2)
    {
        assert(Addr1 && Addr2);
        if (Addr1->sa_family == AF_INET) {
            if (Addr2->sa_family != AF_INET) {
                return false;
            }
            auto R1 = (sockaddr_in *)Addr1;
            auto R2 = (sockaddr_in *)Addr2;
            return (0 == memcmp(&R1->sin_addr, &R2->sin_addr, sizeof(sockaddr_in::sin_addr))
                && R1->sin_port == R2->sin_port);
        }
        else if (Addr1->sa_family == AF_INET6) {
            if (Addr2->sa_family != AF_INET6) {
                return false;
            }
            auto R1 = (sockaddr_in6 *)Addr1;
            auto R2 = (sockaddr_in6 *)Addr2;
            return (0 == memcmp(&R1->sin6_addr, &R2->sin6_addr, sizeof(sockaddr_in6::sin6_addr))
                && R1->sin6_port == R2->sin6_port);
        }
        return false;
    }

    bool xRsmAddr::From(const char * IpStr, uint16_t Port)
    {
        memset(&SockAddrStorage, 0, sizeof(SockAddrStorage));
        if (1 == evutil_inet_pton(AF_INET, IpStr, &Ipv4.sin_addr))
        {
            Ipv4.sin_family = AF_INET;
            Ipv4.sin_port = htons(Port);
            return true;
        }
        else if (1 == evutil_inet_pton(AF_INET6, IpStr, &Ipv6.sin6_addr))
        {
            Ipv6.sin6_family = AF_INET6;
            Ipv6.sin6_port = htons(Port);
            return true;
        }
        return false;
    }

}