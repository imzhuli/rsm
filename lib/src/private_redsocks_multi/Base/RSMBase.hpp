#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>

#include <redsocks_multi/RSM.hpp>
#include <redsocks_multi/RSMConfig.hpp>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <atomic>

ZEC_NS
{
    class xRsmTimeoutNode : public xListNode {};
    using xRsmTimeoutList = xList<xRsmTimeoutNode>;

    class xRsmDeleteNode : public xListNode, xAbstract {};
    using xRsmDeleteList = xList<xRsmDeleteNode>;

    class xRsmAddr
    {
    public:
        union {
            sockaddr          SockAddr = {};
            sockaddr_in       Ipv4;
            sockaddr_in6      Ipv6;
            sockaddr_storage  SockAddrStorage;
        };

        ZEC_INLINE bool IsIpv4() const { return SockAddr.sa_family == AF_INET; }
        ZEC_INLINE bool IsIpv6() const { return SockAddr.sa_family == AF_INET6; }

        ZEC_INLINE sockaddr * GetSockAddr() { return &SockAddr; }
        ZEC_INLINE const sockaddr * GetSockAddr() const { return &SockAddr; }
        ZEC_INLINE size_t GetSockAddrLen() const { return IsIpv4() ? sizeof(Ipv4) : (IsIpv6() ? sizeof(Ipv6) : 0); }
        ZEC_API_MEMBER bool From(const char * IpStr, uint16_t Port);
    };

    ZEC_PRIVATE bool RSM_TestEqual(const sockaddr * Addr1, const sockaddr * Addr2);

#ifdef ZEC_LIBEVENT_MT
    using StatisticCounter = std::atomic_uint64_t;
#else
    using StatisticCounter = uint64_t;
#endif

    ZEC_PRIVATE StatisticCounter TotalIpOnlySourceRules;

    ZEC_PRIVATE StatisticCounter PeriodConnections;
    ZEC_PRIVATE StatisticCounter PeriodClientError;
    ZEC_PRIVATE StatisticCounter PeriodProxyError;
    ZEC_PRIVATE StatisticCounter PeriodClientClose;
    ZEC_PRIVATE StatisticCounter PeriodProxyClose;
    ZEC_PRIVATE StatisticCounter PeriodDataIn;
    ZEC_PRIVATE StatisticCounter PeriodDataOut;
    ZEC_PRIVATE StatisticCounter TotalConnections;

    ZEC_INLINE auto RSM_GetAndReset(StatisticCounter & Counter) {
    #ifdef ZEC_LIBEVENT_MT
        return Counter.exchange(0);
    #else
        return Steal(Counter);
    #endif
    }

}
