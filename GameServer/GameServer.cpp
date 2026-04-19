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

//void Test()
//{
//    const int32 ITERATIONS = 100'000; // 10만 번 반복
//
//    // --- 1. 일반 STL Allocator (malloc/free) 테스트 ---
//    {
//        auto start = std::chrono::high_resolution_clock::now();
//
//        for (int32 i = 0; i < ITERATIONS; ++i)
//        {
//            // 매번 힙 할당과 해제가 발생
//            std::vector<int32> v;
//            for (int32 j = 0; j < 100; ++j)
//                v.push_back(j);
//        }
//
//        auto end = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> elapsed = end - start;
//        std::cout << "Standard Allocator: " << elapsed.count() << "s" << std::endl;
//    }
//
//    // --- 2. Frame Allocator 테스트 ---
//    {
//        // 테스트를 위해 임시로 TLS 할당자 초기화
//        CoreTLS::OnThreadStart(ThreadType::LOGIC, 1);
//
//        auto start = std::chrono::high_resolution_clock::now();
//
//        for (int32 i = 0; i < ITERATIONS; ++i)
//        {
//            // 포인터만 밀어서 할당 (속도가 매우 빠름)
//            FrameVector<int32> v;
//            for (int32 j = 0; j < 100; ++j)
//                v.push_back(j);
//
//            // 프레임 끝에서 초기화 (실제로는 메인루프 끝에서 호출)
//            LFrameAllocator->Clear();
//        }
//
//        auto end = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> elapsed = end - start;
//        std::cout << "Frame Allocator:    " << elapsed.count() << "s" << std::endl;
//
//        CoreTLS::OnThreadEnd();
//    }
//}

int main()
{
	SYSTEM_INFO info;
	::GetSystemInfo(&info);

	info.dwPageSize;
	info.dwAllocationGranularity;

	std::cout << std::endl;

	/*CoreGlobal::Init();

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

	std::shared_ptr<Session> session;
	
	CoreGlobal::Clear();*/
}
