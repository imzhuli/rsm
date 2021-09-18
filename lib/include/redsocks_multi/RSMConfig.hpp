#pragma once
#include <zec/Common.hpp>
#include <string>

ZEC_NS
{

	static inline constexpr const int RsmTcpKeepaliveTime = 7200;
	static inline constexpr const int RsmTcpKeepaliveProbes = 9;
	static inline constexpr const int RsmTcpKeepaliveInterval = 75;
    static inline constexpr const uint64_t RsmProxyExpire = 300;

    enum struct xRsmRuleType // the larger the value is, the higher priority the Rule is of
    {
        IpOnlySourceRule  = 0,
        IpOnlyTargetRule  = 1,
        ExactSourceRule   = 2,
        ExactTargetRule   = 3,
    };

    struct xRsmConfig
    {
        std::string ConfigIp   = "192.168.100.1";
        uint16_t    ConfigPort = 22222;

        std::string EntryIp    = "192.168.100.1";
        uint16_t    EntryPort  = 11111;

        uint32_t    ProxyExpire = RsmProxyExpire;

        bool EnableLibeventMT = false;
        bool IpOnlySourceRule = true;
        bool ExactSourceRule = false;
        bool IpOnlyTargetRule = false;
        bool ExactTargetRule = false;
    };

}
