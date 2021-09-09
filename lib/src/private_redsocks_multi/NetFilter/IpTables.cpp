#include <redsocks_multi/NetFilter/IpTables.hpp>

#ifdef ZEC_SYSTEM_LINUX

#include <linux/netfilter_ipv4.h>

ZEC_NS
{

    bool GetOriginalTarget(int fd, xRef<sockaddr_in> TargetAddrOutput)
    {
        auto & Output = TargetAddrOutput.Get();
        socklen_t socklen = sizeof(Output);
        int error = getsockopt(fd, SOL_IP, SO_ORIGINAL_DST, &Output, &socklen);
        if (error) {
            return false;
        }
        return true;
    }
}

#endif
