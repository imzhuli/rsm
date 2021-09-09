#pragma once
#include <zec/Common.hpp>

#ifdef ZEC_SYSTEM_LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

ZEC_NS
{
    
    ZEC_API bool GetOriginalTargetAddr(int fd, xRef<sockaddr_in> Output);

}


#endif