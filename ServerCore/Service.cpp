#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Listener.h"

Service::Service(NetAddress address, SessionFactory factory, int32 maxSessionCount)
	: _address(address)
	, _sessionFactory(factory)
	, _maxSessionCount(maxSessionCount)
{}

SessionRef Service::CreateSession()
{
    SessionRef session = _sessionFactory();
    session->SetService(shared_from_this());
    return session;
}

void Service::AddSession(SessionRef session)
{
    std::lock_guard<std::mutex> guard(_lock);
    _sessionCount++;
    _sessions.insert(session);
}

void Service::ReleaseSession(SessionRef session)
{
    std::lock_guard<std::mutex> guard(_lock);
    _sessionCount--;
    _sessions.erase(session);
}

/*--------------
  ServerService
--------------*/
ServerService::ServerService(NetAddress address, SessionFactory factory, int32 maxSessionCount)
    : Service(address, factory, maxSessionCount)
{
}

bool ServerService::Start()
{
    _listener = MakeShared<Listener>();
    return _listener->StartAccept(_address, shared_from_this());
}

ClientService::ClientService(NetAddress address, SessionFactory factory, int32 maxSessionCount) : Service(address, factory, maxSessionCount)
{}

bool ClientService::Start()
{
    for (int32 i = 0; i < GetMaxSessionCount(); i++)
    {
        Connect();
    }
    return true;
}

void ClientService::Connect()
{
    SessionRef session = CreateSession();
    if (!GIocpCore->Register(session))
    {
        LOG_ERROR("ClientService: IOCP register failed");
        return;
    }
    session->RegisterConnect(GetAddress());
}
