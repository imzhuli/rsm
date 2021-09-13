#pragma once
#include "./Base/RSMBase.hpp"
#include "./RSMConfig.hpp"
#include "./RSMS5.hpp"
#include <zec/Util/Chrono.hpp>
#include <string>

ZEC_NS
{

	struct xRsmS5Proxy
	{
		xRsmAddr Addr;
		xOptional<xRsmProxyAuth> Auth;
	};

    struct xRsmRule
    {
        xRsmS5Proxy Sock5Proxy;
        union {
            uint64_t    Timeout = 60;
            uint64_t    EndTime;
        };
    };

    ZEC_PRIVATE std::string RSM_MakeExactAddressKey(const sockaddr * AddrPtr);
    ZEC_PRIVATE std::string RSM_MakeIpOnlyAddressKey(const sockaddr * AddrPtr);

    ZEC_PRIVATE const xRsmRule * RSM_GetProxyRule(const sockaddr * SourceAddrPtr, const sockaddr * TargetAddrPtr);
    ZEC_PRIVATE bool RSM_SetProxyRule(xRsmRule && Rule, const sockaddr * MatchAddr, xRsmRuleType Type = xRsmRuleType::IpOnlySourceRule);
    ZEC_PRIVATE void RSM_UnsetProxyRule(const sockaddr * MatchAddr, xRsmRuleType Type = xRsmRuleType::IpOnlySourceRule);
    ZEC_PRIVATE void RSM_ClearTimeoutRules();

    ZEC_PRIVATE void RSM_SetProxyBlacklist(const sockaddr * ProxyAddr);
    ZEC_PRIVATE void RSM_UnsetProxyBlacklist(const sockaddr * ProxyAddr);
    ZEC_PRIVATE bool RSM_IsInBlacklist(const sockaddr * ProxyAddr);
    ZEC_PRIVATE void RSM_ClearBlacklist();
}
