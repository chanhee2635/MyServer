#include "pch.h"
#include "Listener.h"
#include "IocpEvent.h"
#include "Session.h"

Listener::~Listener()
{
	SocketUtils::Close(_listenSocket);

	for (AcceptEvent* event : _acceptEvents)
		delete event;
}

bool Listener::StartAccept(NetAddress address, int32 acceptCount)
{
	_listenSocket = SocketUtils::CreateSocket();
	if (_listenSocket == INVALID_SOCKET) return false;

	if (GIocpCore.Register(shared_from_this()) == false) return false;

	if (SocketUtils::SetReuseAddress(_listenSocket, true) == false) return false;
	if (SocketUtils::SetLinger(_listenSocket, 0, 0) == false) return false;

	if (SocketUtils::Bind(_listenSocket, address) == false) return false;
	if (SocketUtils::Listen(_listenSocket) == false) return false;

	for (int32 i = 0; i < acceptCount; ++i)
	{
		AcceptEvent* acceptEvent = new AcceptEvent();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

void Listener::Close()
{
	SocketUtils::Close(_listenSocket);
	_listenSocket = INVALID_SOCKET;
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	ASSERT_CRASH(iocpEvent->eventType == IocpEventType::Accept);
	ProcessAccept(static_cast<AcceptEvent*>(iocpEvent));
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	acceptEvent->Init();
	acceptEvent->owner = shared_from_this();

	SessionRef session = std::make_shared<Session>();
	acceptEvent->session = session;
	
	DWORD bytesReceived = 0;
	if (SocketUtils::AcceptEx(_listenSocket, session->GetSocket(), acceptEvent->addressBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT &bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)) == false)
	{
		const int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			//session->HandleError(errCode);
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = acceptEvent->session;

	if (SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _listenSocket) == false)
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN* localAddr = nullptr;
	SOCKADDR_IN* remoteAddr = nullptr;
	int32 localLen = 0;
	int32 remoteLen = 0;
	::GetAcceptExSockaddrs(acceptEvent->addressBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, reinterpret_cast<SOCKADDR**>(&localAddr), &localLen, reinterpret_cast<SOCKADDR**>(&remoteAddr), &remoteLen);
	if (remoteAddr)
		session->SetNetAddress(NetAddress(*remoteAddr));

	//session->ProcessConnect();

	RegisterAccept(acceptEvent);
}
