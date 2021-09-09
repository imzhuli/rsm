#pragma once
#include <zec/Common.hpp>
#include <string>

ZEC_NS
{

    struct xRsmConfig
    {
        std::string ConfigIp   = "0.0.0.0";
        uint16_t    ConfigPort = 12345;

        std::string EntryIp    = "0.0.0.0";
        uint16_t    EntryPort  = 7788;
    };

}
