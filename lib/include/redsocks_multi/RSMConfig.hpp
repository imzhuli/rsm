#pragma once
#include <zec/Common.hpp>
#include <string>

ZEC_NS
{

	static inline constexpr const int RsmTcpKeepaliveTime = 7200;
	static inline constexpr const int RsmTcpKeepaliveProbes = 9;
	static inline constexpr const int RsmTcpKeepaliveInterval = 75;
    static inline constexpr const uint64_t RsmProxyExpire = 300;

    struct xRsmConfig
    {
        std::string ConfigIp   = "192.168.100.1";
        uint16_t    ConfigPort = 22222;

        std::string EntryIp    = "192.168.100.1";
        uint16_t    EntryPort  = 11111;

        uint32_t    ProxyExpire = RsmProxyExpire;

        bool IpOnlySourceRule = true;
    };

}
