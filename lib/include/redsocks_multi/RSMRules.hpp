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
            sockaddr       Header = {};            
            sockaddr_in    Ipv4;
            sockaddr_in6   Ipv6;
        };

        ZEC_INLINE bool IsIpv4() const { return Header.sa_family == AF_INET; }
        ZEC_INLINE bool IsIpv6() const { return Header.sa_family == AF_INET6; }
    };

	struct xRsmProxyAuth
	{
		std::string Username;
		std::string Password;
	};
	
	struct xRsmProxyS5
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

}
