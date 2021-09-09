#pragma once
#include <zec/Common.hpp>
#include <event2/event.h>

ZEC_NS
{

    struct xRsmConnection
    {
        enum eState {
            Inited = 0,
            Accepted,
            ConnectingToProxy,
            WaitingAuthorization,
            AuthorizationDone,
            AuthorizationRejected,
            Closed,
        };

        
        eState State; 

    };

}

