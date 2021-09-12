#pragma once
#include "./Base/RSMBase.hpp"
#include "./RSMConfig.hpp"
#include <zec/Util/Chrono.hpp>
#include <string>

ZEC_NS
{

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
        xRsmS5Proxy Sock5Proxy;
        union {
            uint64_t    Timeout = 600;
            uint64_t    EndTime;
        };
    };

    ZEC_PRIVATE std::string RSM_MakeExactAddressKey(const sockaddr * AddrPtr);    
    ZEC_PRIVATE std::string RSM_MakeIpOnlyAddressKey(const sockaddr * AddrPtr);

    ZEC_PRIVATE const xRsmRule * RSM_GetProxyRule(const sockaddr * SourceAddrPtr, const sockaddr * TargetAddrPtr);
    ZEC_PRIVATE bool RSM_SetProxyRule(xRsmRule && Rule, const sockaddr * MatchAddr, xRsmRuleType Type = xRsmRuleType::IpOnlySourceRule);
    ZEC_PRIVATE void RSM_UnsetProxyRule(const sockaddr * MatchAddr, xRsmRuleType Type = xRsmRuleType::IpOnlySourceRule);
    ZEC_PRIVATE void RSM_ClearTimeoutRules();
}
