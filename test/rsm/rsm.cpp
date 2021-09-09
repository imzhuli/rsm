#include <redsocks_multi/RSM.hpp>
using namespace zec;

#include <zec/Util/Chrono.hpp>
#include <thread>

static xSimpleLogger Logger;

int main(int, char **) 
{
    auto Guard = xResourceGuard{ Logger };
    
    xRsmConfig Config {};
    RSM_Init(Config, &Logger);

    RSM_LogI("Is RSM Ready ? %s", YN(RSM_IsReady()));
    RSM_Run();
    RSM_Clean();
}
