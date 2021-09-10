#pragma once
#include <zec/Common.hpp>
#include <string>
#include <event2/event.h>
#include <event2/util.h>

ZEC_NS
{

    enum struct xRsmRuleType : uint8_t
    {
        Unspecified = 0,
        SourceMatch = 1,
        TargetMatch = 2,
        MatchBoth   = 3,
    };

    struct xRsmAddr
    {
        union {
            sockaddr       SockAddrHeader = {};            
            sockaddr_in    Ipv4;
            sockaddr_in6   Ipv6;
        };

        ZEC_INLINE bool IsIpv4() const { return SockAddrHeader.sa_family == AF_INET; }
        ZEC_INLINE bool IsIpv6() const { return SockAddrHeader.sa_family == AF_INET6; }

        ZEC_INLINE sockaddr * GetAddr() { return &SockAddrHeader; }
        ZEC_INLINE const sockaddr * GetAddr() const { return &SockAddrHeader; }
        ZEC_INLINE size_t GetAddrLen() const { return IsIpv4() ? sizeof(Ipv4) : (IsIpv6() ? sizeof(Ipv6) : 0); }
    };

	struct xRsmProxyAuth
	{
		std::string Username;
		std::string Password;
	};
	
	struct xRsmS5Proxy
	{
		xRsmAddr Addr;
		xOptional<xRsmProxyAuth> Auth;
	};

    struct xRsmRule
    {
        xRsmRuleType Type;
        xRsmAddr SourceAddr;
        xRsmAddr TargetAddr;
        xRsmAddr Sock5ProxyAddr;
    };
        
    struct xRsmRequest
    {
        xOptional<std::string> SourceIp;
        xOptional<std::string> TargetIp;
        xOptional<uint16_t>    TargetPort;
        
        std::string            S5ProxyIp;
        uint16_t               S5ProxyPort;
    };

}
