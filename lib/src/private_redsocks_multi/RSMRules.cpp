#include "./RSMRules.hpp"
#include <redsocks_multi/RSM.hpp>
#include <zec/String.hpp>
#include <zec/Util/Chrono.hpp>
#include <unordered_map>

ZEC_NS
{
    using xProxyRuleMap = std::unordered_map<std::string, xRsmRule>;
    static_assert(sizeof(sockaddr_in6) == sizeof(xRsmAddr));

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
    
    static xProxyRuleMap ExactTargetRuleMap;
    static xProxyRuleMap IpOnlyTargetRuleMap;
    static xProxyRuleMap ExactSourceRuleMap;
    static xProxyRuleMap IpOnlySourceRuleMap;

    const xRsmRule * RSM_GetProxyRule(const sockaddr * SourceAddrPtr, const sockaddr * TargetAddrPtr)
    {
        auto & Config = RSM_GetConfig();
        if (Config.ExactTargetRule) {
            auto Key = RSM_MakeExactAddressKey(TargetAddrPtr);
            auto Iter = ExactTargetRuleMap.find(Key);
            if (Iter != ExactTargetRuleMap.end()) {
                return &Iter->second;
            }
        }
        if (Config.ExactSourceRule) {
            auto Key = RSM_MakeExactAddressKey(SourceAddrPtr);
            auto Iter = ExactSourceRuleMap.find(Key);
            if (Iter != ExactSourceRuleMap.end()) {
                return &Iter->second;
            }
        }
        if (Config.IpOnlyTargetRule) {
            auto Key = RSM_MakeIpOnlyAddressKey(TargetAddrPtr);
            auto Iter = IpOnlyTargetRuleMap.find(Key);
            if (Iter != IpOnlyTargetRuleMap.end()) {
                return &Iter->second;
            }
        }
        if (Config.IpOnlySourceRule) {
            auto Key = RSM_MakeIpOnlyAddressKey(SourceAddrPtr);
            auto Iter = IpOnlySourceRuleMap.find(Key);
            if (Iter != IpOnlySourceRuleMap.end()) {
                return &Iter->second;
            }
        }
        return nullptr;
    }

    bool RSM_SetProxyRule(xRsmRule && Rule, const sockaddr * MatchAddr, xRsmRuleType Type)
    {
        if (Rule.Timeout != (uint64_t)(-1)) {
            Rule.EndTime = Rule.Timeout + GetTimestamp();
        }
        if (Type == xRsmRuleType::IpOnlySourceRule) {
            auto Key = RSM_MakeIpOnlyAddressKey(MatchAddr);
            IpOnlySourceRuleMap.insert_or_assign(Key, std::move(Rule));
            return true;
        }
        else if (Type == xRsmRuleType::IpOnlyTargetRule) {
            auto Key = RSM_MakeIpOnlyAddressKey(MatchAddr);
            IpOnlyTargetRuleMap.insert_or_assign(Key, std::move(Rule));
            return true;
        }
        else if (Type == xRsmRuleType::ExactSourceRule) {
            auto Key = RSM_MakeExactAddressKey(MatchAddr);
            ExactSourceRuleMap.insert_or_assign(Key, std::move(Rule));
            return true;
        }
        else if (Type == xRsmRuleType::ExactTargetRule) {
            auto Key = RSM_MakeExactAddressKey(MatchAddr);
            ExactTargetRuleMap.insert_or_assign(Key, std::move(Rule));
            return true;
        }
        return false;
    }

    void RSM_UnsetProxyRule(const sockaddr * MatchAddr, xRsmRuleType Type)
    {
        if (Type == xRsmRuleType::IpOnlySourceRule) {
            auto Key = RSM_MakeIpOnlyAddressKey(MatchAddr);
            auto Iter = IpOnlySourceRuleMap.find(Key);
            if (Iter == IpOnlySourceRuleMap.end()) {
                return;
            }
            IpOnlySourceRuleMap.erase(Iter);
        }
        else if (Type == xRsmRuleType::IpOnlyTargetRule) {
            auto Key = RSM_MakeIpOnlyAddressKey(MatchAddr);
            auto Iter = IpOnlyTargetRuleMap.find(Key);
            if (Iter == IpOnlyTargetRuleMap.end()) {
                return;
            }
            IpOnlyTargetRuleMap.erase(Iter);
        }
        else if (Type == xRsmRuleType::ExactSourceRule) {
            auto Key = RSM_MakeExactAddressKey(MatchAddr);
            auto Iter = ExactSourceRuleMap.find(Key);
            if (Iter == ExactSourceRuleMap.end()) {
                return;
            }
            ExactSourceRuleMap.erase(Iter);
        }
        else if (Type == xRsmRuleType::ExactTargetRule) {
            auto Key = RSM_MakeExactAddressKey(MatchAddr);
            auto Iter = ExactTargetRuleMap.find(Key);
            if (Iter == ExactTargetRuleMap.end()) {
                return;
            }
            ExactTargetRuleMap.erase(Iter);
        }
    }

    static xTimer ClearTimer;
    void RSM_ClearTimeoutRules()
    {
        if (!ClearTimer.TestAndTag(1s)) {
            return;
        }
        uint64_t Timestamp = GetTimestamp();
        switch(Timestamp % 4) {
            case 0: {
                for (auto Iter = IpOnlySourceRuleMap.begin(); Iter != IpOnlySourceRuleMap.end();) {
                    if (Iter->second.EndTime < Timestamp) {
                        Iter = IpOnlySourceRuleMap.erase(Iter);
                    } else {
                        ++Iter;
                    }
                }
                break;
            }
            case 1: {
                for (auto Iter = ExactSourceRuleMap.begin(); Iter != ExactSourceRuleMap.end();) {
                    if (Iter->second.EndTime < Timestamp) {
                        Iter = ExactSourceRuleMap.erase(Iter);
                    } else {
                        ++Iter;
                    }
                }
                break;
            }
            case 2: {
                for (auto Iter = IpOnlyTargetRuleMap.begin(); Iter != IpOnlyTargetRuleMap.end();) {
                    if (Iter->second.EndTime < Timestamp) {
                        Iter = IpOnlyTargetRuleMap.erase(Iter);
                    } else {
                        ++Iter;
                    }
                }
                break;
            }
            case 3: {
                for (auto Iter = ExactTargetRuleMap.begin(); Iter != ExactTargetRuleMap.end();) {
                    if (Iter->second.EndTime < Timestamp) {
                        Iter = ExactTargetRuleMap.erase(Iter);
                    } else {
                        ++Iter;
                    }
                }
                break;
            }
        }
    }

}

