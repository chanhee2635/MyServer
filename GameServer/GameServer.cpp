#include "pch.h"
#include <iostream>

#include "IocpCore.h"
#include "Listener.h"
#include "ThreadManager.h"

struct TestObject
{
    int32 x, y, z;
    TestObject(int32 x, int32 y, int32 z) : x(x), y(y), z(z) {}
};

void PoolTest()
{
    std::cout << "=== Pool Allocator Test ===\n";

    // 기본 할당/해제
    TestObject* obj = xnew<TestObject>(1, 2, 3);
    std::cout << "xnew: " << obj->x << ", " << obj->y << ", " << obj->z << "\n";
    xdelete(obj);

    // 풀 재사용 확인 (해제 후 같은 주소 반환되는지)
    TestObject* a = xnew<TestObject>(10, 20, 30);
    void* addrA = a;
    xdelete(a);

    TestObject* b = xnew<TestObject>(40, 50, 60);
    void* addrB = b;
    std::cout << "Pool reuse: " << (addrA == addrB ? "YES" : "NO") << "\n";
    xdelete(b);

    // shared_ptr
    auto sp = MakeShared<TestObject>(7, 8, 9);
    std::cout << "MakeShared: " << sp->x << ", " << sp->y << ", " << sp->z << "\n";
}

void PerformanceTest()
{
    std::cout << "\n=== Performance Test (100,000 iterations) ===\n";
    const int32 COUNT = 100'000;

    // new/delete
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int32 i = 0; i < COUNT; ++i)
        {
            TestObject* obj = new TestObject(i, i, i);
            delete obj;
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "new/delete : "
            << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
            << " us\n";
    }

    // xnew/xdelete
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int32 i = 0; i < COUNT; ++i)
        {
            TestObject* obj = xnew<TestObject>(i, i, i);
            xdelete(obj);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "xnew/xdelete: "
            << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
            << " us\n";
    }
}

void FrameAllocatorTest()
{
    std::cout << "\n=== Frame Allocator Test ===\n";
    CoreTLS::OnThreadStart(ThreadType::LOGIC, 1);

    {
        FrameVector<int32> v;
        for (int32 i = 0; i < 10; ++i)
            v.push_back(i);

        std::cout << "FrameVector: ";
        for (int32 val : v) std::cout << val << " ";
        std::cout << "\n";
    }

    LFrameAllocator->Clear();
    CoreTLS::OnThreadEnd();
}

void MultiThreadTest()
{
    std::cout << "\n=== Multi-Thread Test (4 threads x 100,000 iterations) ===\n";
    const int32 THREAD_COUNT = 4;
    const int32 ITER_PER_THREAD = 100'000;

    // new/delete (OS 힙)
    {
        std::vector<std::thread> threads;
        std::atomic<long long> totalTime = 0;

        for (int32 t = 0; t < THREAD_COUNT; ++t)
        {
            threads.emplace_back([&]()
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    for (int32 i = 0; i < ITER_PER_THREAD; ++i)
                    {
                        TestObject* obj = new TestObject(i, i, i);
                        delete obj;
                    }
                    auto end = std::chrono::high_resolution_clock::now();
                    totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                });
        }
        for (auto& th : threads) th.join();
        std::cout << "new/delete       (OS heap)  : " << totalTime / THREAD_COUNT << " us/thread\n";
    }

    // xnew/xdelete — Global Pool만 (TLS 없음)
    {
        std::vector<std::thread> threads;
        std::atomic<long long> totalTime = 0;

        for (int32 t = 0; t < THREAD_COUNT; ++t)
        {
            threads.emplace_back([&]()
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    for (int32 i = 0; i < ITER_PER_THREAD; ++i)
                    {
                        TestObject* obj = xnew<TestObject>(i, i, i);
                        xdelete(obj);
                    }
                    auto end = std::chrono::high_resolution_clock::now();
                    totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                });
        }
        for (auto& th : threads) th.join();
        std::cout << "xnew/xdelete     (Global)   : " << totalTime / THREAD_COUNT << " us/thread\n";
    }

    // xnew/xdelete — TLS Pool 활성화
    {
        std::vector<std::thread> threads;
        std::atomic<long long> totalTime = 0;
        std::atomic<int32> threadId = 0;

        for (int32 t = 0; t < THREAD_COUNT; ++t)
        {
            threads.emplace_back([&]()
                {
                    CoreTLS::OnThreadStart(ThreadType::LOGIC, threadId++);

                    auto start = std::chrono::high_resolution_clock::now();
                    for (int32 i = 0; i < ITER_PER_THREAD; ++i)
                    {
                        TestObject* obj = xnew<TestObject>(i, i, i);
                        xdelete(obj);
                    }
                    auto end = std::chrono::high_resolution_clock::now();
                    totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

                    CoreTLS::OnThreadEnd();
                });
        }
        for (auto& th : threads) th.join();
        std::cout << "xnew/xdelete     (TLS Pool) : " << totalTime / THREAD_COUNT << " us/thread\n";
    }
}

int main()
{
    CoreGlobal::Init();

    MultiThreadTest();

    CoreGlobal::Clear();
}
