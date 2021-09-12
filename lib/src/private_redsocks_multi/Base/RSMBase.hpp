#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <atomic>

ZEC_NS
{
    class xRsmTimeoutNode : public xListNode {};
    using xRsmTimeoutList = xList<xRsmTimeoutNode>;
    
    class xRsmDeleteNode : public xListNode, xAbstract {};
    using xRsmDeleteList = xList<xRsmDeleteNode>;
    
    struct xRsmAddr
    {
        union {
            sockaddr       SockAddrBase = {};            
            sockaddr_in    Ipv4;
            sockaddr_in6   Ipv6;
        };

        ZEC_INLINE bool IsIpv4() const { return SockAddrBase.sa_family == AF_INET; }
        ZEC_INLINE bool IsIpv6() const { return SockAddrBase.sa_family == AF_INET6; }

        ZEC_INLINE sockaddr * GetAddr() { return &SockAddrBase; }
        ZEC_INLINE const sockaddr * GetAddr() const { return &SockAddrBase; }
        ZEC_INLINE size_t GetAddrLen() const { return IsIpv4() ? sizeof(Ipv4) : (IsIpv6() ? sizeof(Ipv6) : 0); }
    };
    
    ZEC_PRIVATE std::atomic_uint64_t TotalExactTargetRules;
    ZEC_PRIVATE std::atomic_uint64_t TotalExactSourceRules;
    ZEC_PRIVATE std::atomic_uint64_t TotalIpOnlyTargetRules;
    ZEC_PRIVATE std::atomic_uint64_t TotalIpOnlySourceRules;
 
    ZEC_PRIVATE std::atomic_uint64_t ConnectionsPerMinute;
    ZEC_PRIVATE std::atomic_uint64_t TotalConnections;
    ZEC_PRIVATE std::atomic_uint64_t TotalDataIn;
    ZEC_PRIVATE std::atomic_uint64_t TotalDataOut;

}
