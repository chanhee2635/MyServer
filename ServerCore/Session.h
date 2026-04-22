#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "RecvBuffer.h"

class Session : public IocpObject
{
private:
	class RecvEvent : public IocpEvent
	{
	public:
		RecvEvent() : IocpEvent(IocpEventType::Recv) {}
	};
	class SendEvent : public IocpEvent
	{
	public:
		SendEvent() : IocpEvent(IocpEventType::Send) {}
	};

public:
	Session();
	virtual ~Session();

	void	   SetNetAddress(const NetAddress& address) { _address = address; }
	void	   SetService(const ServiceRef& service) { _service = service; }
	SessionRef GetRef() { return std::static_pointer_cast<Session>(shared_from_this()); }
	NetAddress GetNetAddress() const { return _address; }
	SOCKET	   GetSocket()	   const { return _socket; }
	ServiceRef GetService()	   const { return _service.lock(); }
	bool	   IsConnected()   const { return _connected; }

	void ProcessConnect();
	void Disconnect();

	virtual HANDLE GetHandle() const override { return reinterpret_cast<HANDLE>(_socket); }
	virtual void   Dispatch(IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

protected:
	virtual void OnConnected()    {}
	virtual void OnDisconnected() {}
	virtual int32 OnRecv(const BYTE* buffer, uint32 len) { return static_cast<int32>(len); }

private:
	void RegisterRecv();
	void ProcessRecv(int32 numOfBytes);

	SOCKET				_socket	   = INVALID_SOCKET;
	NetAddress			_address   = {};
	std::atomic<bool>	_connected = false;
	ServiceWeakRef		_service;

	RecvEvent  _recvEvent;
	SendEvent  _sendEvent;
	RecvBuffer _recvBuffer;
};

