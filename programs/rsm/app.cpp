#include <redsocks_multi/RSM.hpp>
#include <redsocks_multi/RSMConfig.hpp>
#include <thread>

#include <zec/Util/Command.hpp>
#include <zec/Util/IniReader.hpp>
#include <iostream>

#include <signal.h>

using namespace zec;
using namespace std;

static xSimpleLogger Logger;

static const xRsmConfig Config;

static void Usage(const char * name = "app_rsm")
{
    cout << name << " " << "[-c ConfigFile]" << endl;
}

int main(int Argc, char **Argv)
{
    zec::xCommandLine CmdLine { Argc, Argv, {
        { 'h', "help", "help", false },
        { 'c', "config", "config", true }
    }};
    if (CmdLine["help"]()) {
        Usage();
        return 0;
    }

    auto Guard = xResourceGuard{ Logger };

    xRsmConfig Config {};
    auto OptConfig = CmdLine["config"];
    if (OptConfig()) {
        xIniReader Reader(OptConfig->c_str());
        auto EntryIpStr = Reader.Get("entry_ip");
        auto EntryPort = Reader.GetInt64("entry_port");
        auto ConfigIpStr = Reader.Get("config_ip");
        auto ConfigPort = Reader.GetInt64("config_port");
        auto ProxyExpire = Reader.GetInt64("proxy_expire");

        if (EntryIpStr) {
            Config.EntryIp = EntryIpStr;
        }
        if (EntryPort > 0) {
            Config.EntryPort = (uint16_t)EntryPort;
        }
        if (ConfigIpStr) {
            Config.ConfigIp = ConfigIpStr;
        }
        if (ConfigPort > 0) {
            Config.ConfigPort = ConfigPort;
        }
        if (ProxyExpire > 0) {
            Config.ProxyExpire = ProxyExpire;
        }
    }
    cout << "Config.EntryIp=\'" << Config.EntryIp << "\'" << endl;
    cout << "Config.EntryPort=\'" << Config.EntryPort << "\'" << endl;
    cout << "Config.ConfigIp=\'" << Config.ConfigIp << "\'" << endl;
    cout << "Config.ConfigPort=\'" << Config.ConfigPort << "\'" << endl;
    cout << "Config.ProxyExpire=\'" << Config.ProxyExpire << "\'" << endl;

    signal(SIGPIPE, SIG_IGN);
    if (!RSM_Init(Config, &Logger)) {
        cerr << "Failed to init service" << endl;
        return -1;
    }
    RSM_Run();
    RSM_Clean();

    return 0;
}

