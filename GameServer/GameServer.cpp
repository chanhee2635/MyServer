#include "pch.h"
#include "Service.h"
#include "GameSession.h"
#include "ThreadManager.h"

int main()
{
    GameGlobal::Init();

    // ServerService 설정
    auto service = MakeShared<ServerService>(
        NetAddress("127.0.0.1", 7777),
        []() { return MakeShared<GameSession>(); },
        100
    );

    ASSERT_CRASH(service->Start());

    // IOCP 워커 스레드
    const uint32 ioThreadCount = std::max(1u, std::thread::hardware_concurrency());
    for (uint32 i = 0; i < ioThreadCount; ++i)
    {
        GThread->Launch(ThreadType::IO, []() {
            while (true)
                GIocpCore->Dispatch();
        });
    }

    // 모니터 스레드
    GThread->Launch(ThreadType::MONITOR, []() {
        ServerStats::Get().Report();
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "\033[7A";
            ServerStats::Get().Report();
        }
        });

    // 메인 스레드 대기
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    GameGlobal::Clear();
}
