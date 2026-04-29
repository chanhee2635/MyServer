#include "pch.h"
#include "DummySession.h"
#include "Service.h"
#include "ThreadManager.h"

int main()
{
    CoreGlobal::Init();

    auto service = MakeShared<ClientService>(
        NetAddress("127.0.0.1", 7777),
        []() { return MakeShared<DummySession>(); },
        2000
    );

    ASSERT_CRASH(service->Start());

    GThread->Launch(ThreadType::IO, []() { while (true) GIocpCore->Dispatch(100); });
    GThread->Launch(ThreadType::IO, []() { while (true) GIocpCore->Dispatch(100); });

    GThread->Launch(ThreadType::LOGIC, [&service]() {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            service->ForEachSession([](SessionRef s) {
                std::static_pointer_cast<DummySession>(s)->SendChat();
            });
        }
    });

    GThread->Join();
    CoreGlobal::Clear();
}