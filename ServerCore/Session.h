#pragma once
#include "IocpCore.h"

class Session : public IocpObject
{
public:
	Session();
	virtual ~Session();

	void SetNetAddress(NetAddress address) { _address = address; }
	NetAddress GetAddress() { return _address; }
	SOCKET GetSocket() { return _socket; }

	virtual HANDLE GetHandle() override { return reinterpret_cast<HANDLE>(_socket); }
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

public:
	char _recvBuffer[1000];

private:
	SOCKET _socket = INVALID_SOCKET;
	NetAddress _address = {};
	std::atomic<bool> _connected = false;
};

