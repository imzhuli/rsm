#pragma once
#include <zec/Common.hpp>
#include <string>

ZEC_NS
{

	static inline constexpr const int TcpKeepaliveTime = 7200;
	static inline constexpr const int TcpKeepaliveProbes = 9;
	static inline constexpr const int TcpKeepaliveInterval = 75;

    struct xRsmConfig
    {
        std::string ConfigIp   = "0.0.0.0";
        uint16_t    ConfigPort = 12345;

        std::string EntryIp    = "0.0.0.0";
        uint16_t    EntryPort  = 7788;
    };

}
