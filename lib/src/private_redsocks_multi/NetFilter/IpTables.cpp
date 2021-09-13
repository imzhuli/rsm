#include "./IpTables.hpp"
#include <redsocks_multi/RSMConfig.hpp>
#include <cstring>

#ifdef ZEC_SYSTEM_LINUX

#include <linux/netfilter_ipv4.h>

ZEC_NS
{
	bool Rsm_GetOriginalTarget(int fd, xRef<xRsmAddr> TargetAddrOutput)
	{
		sockaddr_storage SockAddr = {};
		socklen_t Socklen = sizeof(SockAddr);
		int error = getsockopt(fd, SOL_IP, SO_ORIGINAL_DST, &SockAddr, &Socklen);
		if (error) {
			return false;
		}
		auto &Output = TargetAddrOutput.Get();
		if(SockAddr.ss_family == AF_INET) {
			memcpy(&Output.Ipv4, &SockAddr, sizeof(Output.Ipv4));
			return true;
		}
		else if(SockAddr.ss_family == AF_INET6) {
			memcpy(&Output.Ipv6, &SockAddr, sizeof(Output.Ipv6));
			return true;
		}
		return false;
	}

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

    bool Rsm_GetOriginalTargetAddrIpv6(int fd, xRef<sockaddr_in6> TargetAddrOutput)
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

	bool Rsm_SetTcpKeepaliveParams(int fd)
	{
		struct
		{
			int level, option, value;
		} opt[] = {
			{SOL_SOCKET,  SO_KEEPALIVE,  1},
			{IPPROTO_TCP, TCP_KEEPIDLE,  RsmTcpKeepaliveTime},
			{IPPROTO_TCP, TCP_KEEPCNT,   RsmTcpKeepaliveProbes},
			{IPPROTO_TCP, TCP_KEEPINTVL, RsmTcpKeepaliveInterval},
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
