#include "pch.h"
#include <iostream>

#include "IocpCore.h"
#include "Listener.h"
#include "ThreadManager.h"

void Job() {
	while (true)
	{
		GIocpCore.Dispatch(1);
	}
}

int main()
{
	CoreGlobal::Init();

	std::shared_ptr<Listener> listener = std::make_shared<Listener>();
	if (listener->StartAccept(NetAddress(L"127.0.0.1", 7777)) == false)
		return 0;

	int iocpThreadCount = std::thread::hardware_concurrency() - 5;
	for (uint8 i = 0; i < iocpThreadCount; ++i)
	{
		GThreadManager->Launch(ThreadType::IO, Job);
	}

	while (true)
	{

	}

	CoreGlobal::Clear();
}
