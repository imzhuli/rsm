#include <redsocks_multi/NetFilter/IpTables.hpp>
#include <redsocks_multi/RSMConfig.hpp>

#ifdef ZEC_SYSTEM_LINUX

#include <linux/netfilter_ipv4.h>

ZEC_NS
{

	bool GetOriginalTargetIpv4(int fd, xRef<sockaddr_in> TargetAddrOutput)
	{
		auto &Output = TargetAddrOutput.Get();
		socklen_t socklen = sizeof(Output);
		int error = getsockopt(fd, SOL_IP, SO_ORIGINAL_DST, &Output, &socklen);
		if (error)
		{
			return false;
		}
		return true;
	}
	
    bool GetOriginalTargetAddrIpv6(int fd, xRef<sockaddr_in6> TargetAddrOutput)
	{
		auto &Output = TargetAddrOutput.Get();
		socklen_t socklen = sizeof(Output);
		int error = getsockopt(fd, SOL_IP, SO_ORIGINAL_DST, &Output, &socklen);
		if (error)
		{
			return false;
		}
		return true;
	}

	bool SetTcpKeepaliveParams(int fd)
	{
		struct
		{
			int level, option, value;
		} opt[] = {
			{SOL_SOCKET,  SO_KEEPALIVE,  1},
			{IPPROTO_TCP, TCP_KEEPIDLE,  TcpKeepaliveTime},
			{IPPROTO_TCP, TCP_KEEPCNT,   TcpKeepaliveProbes},
			{IPPROTO_TCP, TCP_KEEPINTVL, TcpKeepaliveInterval},
		};
		for (size_t i = 0; i < Length(opt); ++i)
		{
			if (opt[i].value)
			{
				if (setsockopt(fd, opt[i].level, opt[i].option, &opt[i].value, sizeof(opt[i].value))) {
					return false;
				}
			}
		}
		return true;
	}
}

#endif
