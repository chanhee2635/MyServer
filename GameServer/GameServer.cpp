#include "pch.h"
#include "Service.h"
#include "GameSession.h"
#include "ThreadManager.h"

int main()
{
    GameGlobal::Init();

    auto service = MakeShared<ServerService>(
        NetAddress("127.0.0.1", 7777),
        []() { return MakeShared<GameSession>(); },
        2000
    );

    ASSERT_CRASH(service->Start());

    GThread->Launch(ThreadType::IO, []() { while (true) GIocpCore->Dispatch(100); });
    GThread->Launch(ThreadType::IO, []() { while (true) GIocpCore->Dispatch(100); });

    GThread->Launch(ThreadType::LOGIC, []() {
        while (true)
        {
            GJobTimer->Distribute(JobTimer::Now());
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    GThread->Launch(ThreadType::LOGIC, []() {
        GGlobalQueue->Distribute();
    });

    GThread->Launch(ThreadType::MONITOR, []() {
        ServerStats::Get().Report();
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "\033[8A";
            ServerStats::Get().Report();
        }
        });

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    GameGlobal::Clear();
}
