#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(IocpEventType::Accept) {}

	void Init() {
		IocpEvent::Init();
		session = nullptr;
	}

public:
	SessionRef session = nullptr;
	alignas(16) char addressBuffer[64] = { 0, };
};

class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

	bool StartAccept(NetAddress address, int32 acceptCount = 10);
	void Close();

	virtual HANDLE GetHandle() override { return reinterpret_cast<HANDLE>(_listenSocket); }
	virtual void Dispatch(IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET _listenSocket = INVALID_SOCKET;
	std::vector<AcceptEvent*> _acceptEvents;
};
