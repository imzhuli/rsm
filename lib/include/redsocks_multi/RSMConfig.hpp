#pragma once
#include <zec/Common.hpp>
#include <string>

ZEC_NS
{

	static inline constexpr const int TcpKeepaliveTime = 7200;
	static inline constexpr const int TcpKeepaliveProbes = 9;
	static inline constexpr const int TcpKeepaliveInterval = 75;

    enum struct xRsmRuleType // the larger the value is, the higher priority the Rule is of
    {
        IpOnlySourceRule  = 0,
        IpOnlyTargetRule  = 1,
        ExactSourceRule   = 2,
        ExactTargetRule   = 3,
    };

    struct xRsmConfig
    {
        std::string ConfigIp   = "0.0.0.0";
        uint16_t    ConfigPort = 22222;

        std::string EntryIp    = "0.0.0.0";
        uint16_t    EntryPort  = 11111;

        bool IpOnlySourceRule = true;
        bool ExactSourceRule = false;
        bool IpOnlyTargetRule = false;
        bool ExactTargetRule = false;
    };

}
