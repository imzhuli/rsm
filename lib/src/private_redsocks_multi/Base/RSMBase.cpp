#include "./RSMBase.hpp"
#include <cstring>

ZEC_NS
{

    static_assert(sizeof(sockaddr_storage) == sizeof(xRsmAddr));

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

    std::string xRsmAddr::ToString() const
    {
        char AddrBuffer[std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
        if (IsIpv4()) {
            inet_ntop(AF_INET, &Ipv4.sin_addr, AddrBuffer, INET_ADDRSTRLEN);
            return std::string{AddrBuffer} + "@" + std::to_string(ntohs(Ipv4.sin_port));
        }
        else if (IsIpv6()) {
            inet_ntop(AF_INET6, &Ipv6.sin6_addr, AddrBuffer, INET6_ADDRSTRLEN);
            return std::string{AddrBuffer} + "@" + std::to_string(ntohs(Ipv6.sin6_port));
        }
        return {};
    }

    bool xRsmAddr::Equals(const xRsmAddr & Other)
    {
        if (SockAddr.sa_family == AF_INET) {
            return !memcmp(&Ipv4, &Other.Ipv4, sizeof(sockaddr_in));
        }
        else if (SockAddr.sa_family == AF_INET6) {
            return !memcmp(&Ipv6, &Other.Ipv6, sizeof(sockaddr_in6));
        }
        return false;
    }

    std::string ToString(const sockaddr * SockaddrPtr)
    {
        char AddrBuffer[std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)] = {};

        if (SockaddrPtr->sa_family == AF_INET) {
            auto Ipv4Ptr = (sockaddr_in*)SockaddrPtr;
            inet_ntop(AF_INET, &Ipv4Ptr->sin_addr, AddrBuffer, INET_ADDRSTRLEN);
            return std::string{AddrBuffer} + "@" + std::to_string(ntohs(Ipv4Ptr->sin_port));
        }
        else if (SockaddrPtr->sa_family == AF_INET6) {
            auto Ipv6Ptr = (sockaddr_in6*)SockaddrPtr;
            inet_ntop(AF_INET6, &Ipv6Ptr->sin6_addr, AddrBuffer, INET6_ADDRSTRLEN);
            return std::string{AddrBuffer} + "@" + std::to_string(ntohs(Ipv6Ptr->sin6_port));
        }
        return {};
    }

}
