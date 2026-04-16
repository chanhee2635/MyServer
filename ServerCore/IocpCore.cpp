#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore GIocpCore;

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
    if(_iocpHandle != INVALID_HANDLE_VALUE)
    	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObjectRef iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, reinterpret_cast<ULONG_PTR>(iocpObject.get()), 0);
}

bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD       numOfBytes = 0;
	IocpObject* iocpObject = nullptr;  // 개선: 키를 사용하여 shared_ptr을 사용하지 않고 바로 접근 
	OVERLAPPED* overlapped = nullptr;

    if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT reinterpret_cast<PULONG_PTR>(&iocpObject), OUT &overlapped, timeoutMs))
    {
        if (iocpObject && overlapped)
            ProcessEvent(iocpObject, static_cast<IocpEvent*>(overlapped), numOfBytes);
    } else {
        const int32 errCode = ::WSAGetLastError();
        if (overlapped)
            ProcessEvent(iocpObject, static_cast<IocpEvent*>(overlapped), numOfBytes);
        else if (errCode != WAIT_TIMEOUT)
            return false;
    }

    return true;
}

void IocpCore::ProcessEvent(IocpObject* obj, IocpEvent* event, int32 bytes)
{
    ASSERT_CRASH(obj != nullptr);
    ASSERT_CRASH(event != nullptr);

    obj->Dispatch(event, bytes);
    event->owner = nullptr;
}

