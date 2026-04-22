#include "pch.h"
#include "Session.h"
#include "Service.h"

Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::ProcessConnect()
{
    _connected = true;

    if (auto service = _service.lock())
        service->AddSession(GetRef());

    OnConnected();
    RegisterRecv();
}

void Session::Disconnect()
{
    if (_connected.exchange(false) == false)
        return;
    
    OnDisconnected();

    if (auto service = _service.lock())
        service->ReleaseSession(GetRef());

    SocketUtils::Close(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
    switch (iocpEvent->GetType())
    {
    case IocpEventType::Recv:
        ProcessRecv(numOfBytes);
        break;
    default:
        break;
    }
}

void Session::RegisterRecv()
{
    if (!_connected) 
        return;

    _recvEvent.Init();
    _recvEvent.SetOwner(shared_from_this());

    auto segment = _recvBuffer.WriteSegment();
    WSABUF wsaBuf;
    wsaBuf.buf = reinterpret_cast<char*>(segment.data());
    wsaBuf.len = static_cast<ULONG>(segment.size());

    DWORD numOfBytes = 0;
    DWORD flags = 0;

    if (::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, &_recvEvent, nullptr) == SOCKET_ERROR)
    {
        int32 errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            _recvEvent.SetOwner(nullptr);
            Disconnect();
        }
    }
}

void Session::ProcessRecv(int32 numOfBytes)
{
    _recvEvent.SetOwner(nullptr);

    if (numOfBytes == 0)
    {
        Disconnect();
        return;
    }

    if (!_recvBuffer.OnWrite(numOfBytes))
    {
        Disconnect();
        return;
    }

    auto segment = _recvBuffer.ReadSegment();
    int32 processLen = OnRecv(segment.data(), static_cast<uint32>(segment.size()));

    if (processLen < 0 || static_cast<uint32>(processLen) > _recvBuffer.GetDataSize())
    {
        Disconnect();
        return;
    }

    _recvBuffer.OnRead(static_cast<uint32>(processLen));
    _recvBuffer.Clean();

    RegisterRecv();
}

