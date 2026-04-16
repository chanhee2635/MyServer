#pragma once
enum class IocpEventType : uint8 { Accept, Connect, Disconnect, Recv, Send };

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(IocpEventType type);

	void		  Init();
	IocpEventType GetType() { return eventType; }

public:
	IocpEventType eventType;
	IocpObjectRef owner;
};

