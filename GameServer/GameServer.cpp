#include "pch.h"
#include <iostream>

#include "IocpCore.h"
#include "Listener.h"

int main()
{
	CoreGlobal::Init();

	std::shared_ptr<Listener> listener = std::make_shared<Listener>();
	if (listener->StartAccept(NetAddress(L"127.0.0.1", 7777)) == false)
		return 0;

	std::cout << "Server Listening on 127.0.0.1:7777..." << std::endl;

	std::vector<std::thread> workerThreads;

	for (int32 i = 0; i < 4; i++)
	{
		workerThreads.emplace_back([=]()
			{
				while (true)
				{
					if (GIocpCore.Dispatch() == false)
						break;
				}
			});
	}

	for (std::thread& t : workerThreads)
	{
		if (t.joinable())
			t.join();
	}

	CoreGlobal::Clear();
}
