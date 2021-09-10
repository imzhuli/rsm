#include <redsocks_multi/RSM.hpp>
#include <redsocks_multi/RSMConfig.hpp>
#include <thread>

#include <zec/Util/Command.hpp>
#include <iostream>

using namespace zec;
using namespace std;

static xSimpleLogger Logger;

static const xRsmConfig Config;

static void Usage(const char * name = "app_rsm")
{
    cout << name << " " << "[-pc ConfigPort]" << endl;
}

int main(int Argc, char **Argv) 
{
    zec::xCommandLine CmdLine { Argc, Argv, {
        { 'h', "help", "help", false }
    }};
    if (CmdLine["help"]()) {
        Usage();
        return 0;
    }

    auto Guard = xResourceGuard{ Logger };
    
    xRsmConfig Config {};
    RSM_Init(Config, &Logger);
    RSM_Run();
    RSM_Clean();

    return 0;
}

