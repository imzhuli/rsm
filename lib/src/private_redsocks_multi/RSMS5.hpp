#pragma once
#include "./Base/RSMBase.hpp"
#include <string>

ZEC_NS
{

    enum struct eRsmS5State
    {
        Init,
        Id_Method_NoAuth,
        Id_Method_UserPass,
        Id_Method_UserPassCheck,
        Id_Method_AuthDone,
        ProxyConnect,

        Ready,
        Closed,
    };

	struct xRsmProxyAuth
	{
		std::string Username;
		std::string Password;
	};

    static constexpr const char S5_Id_Method_NoAuth[] = {
        '\x05',
        '\x01',
        '\x00', // no auth
    };
    static constexpr const char S5_Id_Method_UserPass[] = {
        '\x05',
        '\x01',
        '\x02', // username/password
    };

    class xS5ProxyDelegate
    : xNonCopyable
    {
    public:
        ZEC_PRIVATE_MEMBER bool OnProxyConnected();
        ZEC_PRIVATE_MEMBER bool OnProxyData();
        ZEC_PRIVATE_MEMBER bool OnClientData();

        ZEC_PRIVATE_MEMBER bool Init(bufferevent * ClientEvent, bufferevent * ProxyEvent, const xRsmAddr & TargetAddr, const xOptional<xRsmProxyAuth> & Auth = {});
        ZEC_PRIVATE_MEMBER bool IsReady();
        ZEC_PRIVATE_MEMBER void Clean();

    private:
        ZEC_PRIVATE_MEMBER void InnerClean();
        ZEC_PRIVATE_MEMBER bool RequestIdMethod();
        ZEC_PRIVATE_MEMBER bool OnIdMethodNoAuthResp();
        ZEC_PRIVATE_MEMBER bool OnIdMethodUserPassResp();
        ZEC_PRIVATE_MEMBER bool OnIdMethodAuthResp();
        ZEC_PRIVATE_MEMBER bool RequestProxyConnect();
        ZEC_PRIVATE_MEMBER bool OnConnectResp();
        ZEC_PRIVATE_MEMBER bool OnProxyPayloadData();
        ZEC_PRIVATE_MEMBER bool RequestPushClientBuffer();


        eRsmS5State               _State = eRsmS5State::Init;
        xOptional<xRsmProxyAuth>  _Auth;
        xRsmAddr                  _TargetAddr;

        bufferevent *             _ProxyEventShadow = nullptr;
        evbuffer *                _ProxyInputShadow = nullptr;
        evbuffer *                _ProxyOutputShadow = nullptr;

        bufferevent *             _ClientEventShadow = nullptr;
        evbuffer *                _ClientInputShadow = nullptr;
        evbuffer *                _ClientOutputShadow = nullptr;
        evbuffer *                _ClientTempBuffer = nullptr;
    };

}
