#pragma once

class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() = 0;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) = 0;
};

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	bool Register(IocpObjectRef iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE);

	HANDLE GetHandle() { return _iocpHandle; }

private:
	void ProcessEvent(IocpObject* obj, IocpEvent* event, int32 bytes);

private:
	HANDLE _iocpHandle;
};

extern IocpCore GIocpCore;
