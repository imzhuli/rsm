#include "./RSMS5.hpp"
#include <zec/Byte.hpp>
#include <zec/String.hpp>

ZEC_NS
{

    bool xS5ProxyDelegate::Init(bufferevent * ClientEvent, bufferevent * ProxyEvent, const xRsmAddr & TargetAddr, const xOptional<xRsmProxyAuth> & Auth)
    {
        assert(ClientEvent && ProxyEvent);
        _ClientEventShadow = ClientEvent;
        _ClientInputShadow = bufferevent_get_input(_ClientEventShadow);
        _ClientOutputShadow = bufferevent_get_output(_ClientEventShadow);

        _ProxyEventShadow = ProxyEvent;
        _ProxyInputShadow = bufferevent_get_input(_ProxyEventShadow);
        _ProxyOutputShadow = bufferevent_get_output(_ProxyEventShadow);

        if (!_ClientInputShadow || !_ClientOutputShadow || !_ProxyInputShadow || !_ProxyOutputShadow) {
            InnerClean();
            return false;
        }
        _Auth = Auth;
        _TargetAddr = TargetAddr;
        _State = eRsmS5State::Init;
        return true;
    }

    void xS5ProxyDelegate::Clean()
    {
        if (_State == eRsmS5State::Closed) {
            return;
        }
        InnerClean();
    }

    void xS5ProxyDelegate::InnerClean()
    {
        if (_ClientTempBuffer) {
            evbuffer_free(Steal(_ClientTempBuffer));
        }
        Reset(_ProxyEventShadow);
        Reset(_ProxyInputShadow);
        Reset(_ProxyOutputShadow);
        _State = eRsmS5State::Closed;
    }

    bool xS5ProxyDelegate::OnProxyConnected()
    {
        assert(_State == eRsmS5State::Init);
        return RequestIdMethod();
    }

    bool xS5ProxyDelegate::OnProxyData()
    {
        switch (_State) {
        case eRsmS5State::Init: {
            Error("Bug!");
            return false;
        }
        case eRsmS5State::Id_Method_NoAuth: {
            return OnIdMethodNoAuthResp();
        }
        case eRsmS5State::Id_Method_UserPass: {
            return OnIdMethodUserPassResp();
        }
        case eRsmS5State::ProxyConnect: {
            return OnConnectResp();
        }
        case eRsmS5State::Ready: {
            return OnProxyPayloadData();
        }
        default:
            break;
        }
        RSM_LogE("InvalidProxyState");
        InnerClean();
        return false;
    }

    bool xS5ProxyDelegate::RequestIdMethod()
    {
        if (!_Auth()) {
            RSM_LogD("ProxyWithNoAuth");
            if (evbuffer_add(_ProxyOutputShadow, S5_Id_Method_NoAuth, Length(S5_Id_Method_NoAuth))) {
                InnerClean();
                return false;
            }
            _State = eRsmS5State::Id_Method_NoAuth;
            return true;
        }

        RSM_LogD("ProxyWithUserPass");
        if (evbuffer_add(_ProxyOutputShadow, S5_Id_Method_UserPass, Length(S5_Id_Method_UserPass))) {
            InnerClean();
            return false;
        }
        _State = eRsmS5State::Id_Method_UserPass;
        return true;
    }

    bool xS5ProxyDelegate::OnIdMethodNoAuthResp()
    {
        ubyte Buffer[2] = {};
        if (evbuffer_get_length(_ProxyInputShadow) < 2) {
            return true;
        }
        evbuffer_remove(_ProxyInputShadow, Buffer, 2);
        if (Buffer[0] != 0x05 || Buffer[1] != 0x00) {
            RSM_LogE("ProxyAuthError");
            InnerClean();
            return false;
        }
        return RequestProxyConnect();
    }

    bool xS5ProxyDelegate::OnIdMethodUserPassResp()
    {
        ubyte Buffer[2] = {};
        if (evbuffer_get_length(_ProxyInputShadow) < 2) {
            return true;
        }
        evbuffer_remove(_ProxyInputShadow, Buffer, 2);

        if (Buffer[0] != 0x05) {
            InnerClean();
            return false;
        }
        if (Buffer[1] == 0x00) {
            return RequestProxyConnect();
        }
        if (Buffer[1] != 0x02) {
            RSM_LogE("Invalid Auth Method");
            InnerClean();
            return false;
        }

        RSM_LogE("Not implemented");
        InnerClean();
        return false;
    }

    bool xS5ProxyDelegate::RequestProxyConnect()
    {
        ubyte Buffer[128];
        xStreamWriter Writer(Buffer);
        Writer.W('\x05');
        Writer.W('\x01');
        Writer.W('\x00');
        if (_TargetAddr.IsIpv4()) {
            Writer.W(0x01);
            Writer.W(&_TargetAddr.Ipv4.sin_addr, 4);
            Writer.W(&_TargetAddr.Ipv4.sin_port, 2);
            static_assert(4 == sizeof(sockaddr_in::sin_addr));
        }
        else if (_TargetAddr.IsIpv6()) {
            Writer.W(0x04);
            Writer.W(&_TargetAddr.Ipv6.sin6_addr, 16);
            Writer.W(&_TargetAddr.Ipv6.sin6_port, 2);
            static_assert(16 == sizeof(sockaddr_in6::sin6_addr));
        }
        if (evbuffer_add(_ProxyOutputShadow, Buffer, (int)Writer.Offset())) {
            InnerClean();
            return false;
        }
        _State = eRsmS5State::ProxyConnect;
        return true;
    }

    bool xS5ProxyDelegate::OnConnectResp()
    {
        ubyte Buffer[4];
        if (evbuffer_copyout(_ProxyInputShadow, Buffer, 4) != 4) {
            return true;
        }
        if (Buffer[0] != '\x05' || Buffer[1] != 0x00) {
            RSM_LogE("ProxyConnectionFailed");
            InnerClean();
            return false;
        }
        if (Buffer[3] == 0x01) {
            // ipv4:
            size_t PayloadSize = 4/*header*/ + 4/*sin_addr*/ + 2/*sin_port*/;
            if (evbuffer_get_length(_ProxyInputShadow) < PayloadSize) {
                return true; // wait until data is enough.
            }
            evbuffer_drain(_ProxyInputShadow, (int)PayloadSize);
            _State = eRsmS5State::Ready;
            return RequestPushClientBuffer();
        }
        else if (Buffer[3] == 0x04) {
            // ipv6:
            size_t PayloadSize = 4/*header*/ + 16/*sin_addr*/ + 2/*sin_port*/;
            if (evbuffer_get_length(_ProxyInputShadow) < PayloadSize) {
                return true; // wait until data is enough.
            }
            evbuffer_drain(_ProxyInputShadow, (int)PayloadSize);
            _State = eRsmS5State::Ready;
            return RequestPushClientBuffer();
        }

        RSM_LogE("UnsupportedTargetAddressType");
        InnerClean();
        return false;
    }

    bool xS5ProxyDelegate::RequestPushClientBuffer()
    {
        if (!_ClientTempBuffer) {
            return true;
        }
        evbuffer_remove_buffer(_ClientTempBuffer, _ProxyOutputShadow, evbuffer_get_length(_ClientTempBuffer));
        evbuffer_free(Steal(_ClientTempBuffer));
        return true;
    }

    bool xS5ProxyDelegate::OnClientData()
    {
        if (_State != eRsmS5State::Ready) {
            if (!_ClientTempBuffer) {
                if(!(_ClientTempBuffer = evbuffer_new())) {
                    InnerClean();
                    return false;
                }
            }
            evbuffer_remove_buffer(_ClientInputShadow, _ClientTempBuffer, evbuffer_get_length(_ClientInputShadow));
            return true;
        }

        assert(!_ClientTempBuffer);
        evbuffer_remove_buffer(_ClientInputShadow, _ProxyOutputShadow, evbuffer_get_length(_ClientInputShadow));
        return true;
    }

    bool xS5ProxyDelegate::OnProxyPayloadData()
    {
        evbuffer_remove_buffer(_ProxyInputShadow, _ClientOutputShadow, evbuffer_get_length(_ProxyInputShadow));
        return true;
    }

}
