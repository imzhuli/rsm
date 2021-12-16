#include "./RSMRules.hpp"
#include <zec/String.hpp>
#include <zec/Util/Chrono.hpp>
#include <unordered_map>
#include <unordered_set>

ZEC_NS
{
    using xProxyRuleMap = std::unordered_map<std::string, xRsmRule>;
    using xProxyRuleBlacklist = std::unordered_set<std::string>;

    std::string RSM_MakeExactAddressKey(const sockaddr * AddrPtr)
    {
        if (AF_INET == AddrPtr->sa_family) {
            auto RealAddrPtr = reinterpret_cast<const sockaddr_in *>(AddrPtr);
            std::string Ret { (const char *)&RealAddrPtr->sin_addr, sizeof(sockaddr_in::sin_addr) };
            Ret.append((const char *)&(uint16_t&)RealAddrPtr->sin_port, 2);
            return Ret;
        }
        else if (AF_INET6 == AddrPtr->sa_family) {
            auto RealAddrPtr = reinterpret_cast<const sockaddr_in6 *>(AddrPtr);
            std::string Ret { (const char *)&RealAddrPtr->sin6_addr, sizeof(sockaddr_in6::sin6_addr ) };
            Ret.append((const char *)&(uint16_t&)RealAddrPtr->sin6_port, 2);
            return Ret;
        }
        return {};
    }

    std::string RSM_MakeIpOnlyAddressKey(const sockaddr * AddrPtr)
    {
        if (AF_INET == AddrPtr->sa_family) {
            auto RealAddrPtr = reinterpret_cast<const sockaddr_in *>(AddrPtr);
            std::string Ret { (const char *)&RealAddrPtr->sin_addr, sizeof(sockaddr_in::sin_addr) };
            Ret.append((const char*)X2Ptr((uint16_t)0), 2);
            return Ret;
        }
        else if (AF_INET6 == AddrPtr->sa_family) {
            auto RealAddrPtr = reinterpret_cast<const sockaddr_in6 *>(AddrPtr);
            std::string Ret { (const char *)&RealAddrPtr->sin6_addr, sizeof(sockaddr_in6::sin6_addr ) };
            Ret.append((const char*)X2Ptr((uint16_t)0), 2);
            return Ret;
        }
        return {};
    }

    static xProxyRuleMap IpOnlySourceRuleMap;

    const xRsmRule * RSM_GetProxyRule(const sockaddr * SourceAddrPtr, const sockaddr * TargetAddrPtr)
    {
        auto & Config = RSM_GetConfig();
        if (Config.IpOnlySourceRule) {
            auto Key = RSM_MakeIpOnlyAddressKey(SourceAddrPtr);
            auto Iter = IpOnlySourceRuleMap.find(Key);
            if (Iter != IpOnlySourceRuleMap.end()) {
                return &Iter->second;
            }
        }
        return nullptr;
    }

    bool RSM_SetProxyRule(xRsmRule && Rule, const sockaddr * MatchAddr)
    {
        if (Rule.Timeout != (uint64_t)(-1)) {
            Rule.EndTime = Rule.Timeout + GetTimestamp();
        }
        if (RSM_IsInBlacklist(Rule.Sock5Proxy.Addr.GetSockAddr())) {
            return false;
        }
        auto Key = RSM_MakeIpOnlyAddressKey(MatchAddr);
        auto [Iter, Insert] = IpOnlySourceRuleMap.insert_or_assign(Key, std::move(Rule));
        if (Insert) {
            ++TotalIpOnlySourceRules;
        }
        return true;
    }

    void RSM_UnsetProxyRule(const sockaddr * MatchAddr)
    {
        auto Key = RSM_MakeIpOnlyAddressKey(MatchAddr);
        auto Iter = IpOnlySourceRuleMap.find(Key);
        if (Iter == IpOnlySourceRuleMap.end()) {
            return;
        }
        IpOnlySourceRuleMap.erase(Iter);
        --TotalIpOnlySourceRules;
    }

    static xTimer ClearTimer;
    void RSM_ClearTimeoutRules()
    {
        if (!ClearTimer.TestAndTag(1s)) {
            return;
        }
        uint64_t Timestamp = GetTimestamp();
        if(!(Timestamp % 5)) {
            for (auto Iter = IpOnlySourceRuleMap.begin(); Iter != IpOnlySourceRuleMap.end();) {
                if (Iter->second.EndTime < Timestamp) {
                    Iter = IpOnlySourceRuleMap.erase(Iter);
                    --TotalIpOnlySourceRules;
                } else {
                    ++Iter;
                }
            }
        }
    }

    static xProxyRuleBlacklist ProxyBlacklist;
    void RSM_SetProxyBlacklist(const sockaddr * ProxyAddr)
    {
        auto Key = RSM_MakeExactAddressKey(ProxyAddr);
        ProxyBlacklist.insert(Key);
    }

    void RSM_UnsetProxyBlacklist(const sockaddr * ProxyAddr)
    {
        auto Key = RSM_MakeExactAddressKey(ProxyAddr);
        auto Iter = ProxyBlacklist.find(Key);
        if (Iter == ProxyBlacklist.end()) {
            return;
        }
        ProxyBlacklist.erase(Iter);
    }

    bool RSM_IsInBlacklist(const sockaddr * ProxyAddr)
    {
        auto Key = RSM_MakeExactAddressKey(ProxyAddr);
        return ProxyBlacklist.end() != ProxyBlacklist.find(Key);
    }

    void RSM_ClearBlacklist()
    {
        ProxyBlacklist.clear();
    }

}

