#pragma once
#include "../Base/RSMBase.hpp"

#ifdef ZEC_SYSTEM_LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

ZEC_NS
{

    ZEC_API bool Rsm_GetOriginalTarget(int fd, xRef<xRsmAddr> TargetAddrOutput);
	ZEC_API bool Rsm_SetTcpKeepaliveParams(int fd);

}

#endif
