#include "pch.h"
#include "Memory.h"
#include "Allocator.h"

MemoryManager::MemoryManager()
{
    for (int32 i = 0; i <= MAX_POOL_SIZE; ++i) _poolTable[i] = nullptr;

    int32 currentSlabSize = 0;
    int32 tableIndex = 1;

    auto CreatePools = [&](int32 endSize, int32 step) {
        for (int32 size = currentSlabSize + step; size <= endSize; size += step)
        {
            MemoryPool* pool = new MemoryPool(size);
            ASSERT_CRASH(_poolCount < POOL_COUNT);
            _pools[_poolCount++] = pool;

            while (tableIndex <= size && tableIndex <= MAX_POOL_SIZE)
                _poolTable[tableIndex++] = pool;

            currentSlabSize = size;
        }
    };

    CreatePools(1024, 32);
    CreatePools(2048, 128);
    CreatePools(4096, 256);

    ASSERT_CRASH(tableIndex == MAX_POOL_SIZE + 1);
}

MemoryManager::~MemoryManager()
{
    for (int32 i = 0; i < _poolCount; i++)
        delete _pools[i];
}

void* MemoryManager::Allocate(uint32 size)
{
#ifdef _STOMP
    return StompAllocator::Allocate(size);
#else
    MemoryHeader* header = nullptr;
    const int32 allocSize = size + sizeof(MemoryHeader);

    if (allocSize > MAX_POOL_SIZE)
    {
        ServerStats::Get().memory.poolMissCount.fetch_add(1, std::memory_order_relaxed);
        header = reinterpret_cast<MemoryHeader*>(BaseAllocator::Allocate(allocSize));
    }
    else if (LThreadMemory != nullptr) [[likely]]
    {
        ServerStats::Get().memory.poolHitCount.fetch_add(1, std::memory_order_relaxed);
        header = LThreadMemory->Allocate(allocSize);
    }
    else
    {
        ServerStats::Get().memory.poolHitCount.fetch_add(1, std::memory_order_relaxed);
        header = _poolTable[allocSize]->Pop();
    }

    ServerStats::Get().memory.liveAllocCount.fetch_add(1, std::memory_order_relaxed);
    return MemoryHeader::AttachHeader(header, allocSize);
#endif
}

void MemoryManager::Release(void* ptr)
{
    if (ptr == nullptr) return;

#ifdef _STOMP
    StompAllocator::Release(ptr);
#else
    ServerStats::Get().memory.liveAllocCount.fetch_sub(1, std::memory_order_relaxed);
    MemoryHeader* header = MemoryHeader::DetachHeader(ptr);
    const int32 allocSize = header->GetAllocSize();

    if (allocSize > MAX_POOL_SIZE)
        BaseAllocator::Release(header);
    else if (LThreadMemory != nullptr) [[likely]]
        LThreadMemory->Release(header);
    else
        _poolTable[allocSize]->Push(header);
#endif
}

// ===================== ThreadLocalMemory =====================

ThreadLocalMemory::ThreadLocalMemory()
{
    MemoryPool*    prevGlobal = nullptr;
    TlsMemoryPool* prevTls    = nullptr;
    int32          poolIndex  = 0;

    for (int32 size = 1; size <= static_cast<int32>(MAX_POOL_SIZE); ++size)
    {
        MemoryPool* gPool = GMemory->GetPool(size);

        if (gPool != prevGlobal)
        {
            prevTls = new TlsMemoryPool(gPool);
            _tlsPools[poolIndex++] = prevTls;
            prevGlobal = gPool;
        }

        _tlsPoolTable[size] = prevTls;
    }
}

ThreadLocalMemory::~ThreadLocalMemory()
{
    for (TlsMemoryPool* pool : _tlsPools)
        delete pool;
}

MemoryHeader* ThreadLocalMemory::Allocate(int32 allocSize)
{
    return _tlsPoolTable[allocSize]->Pop();
}

void ThreadLocalMemory::Release(MemoryHeader* header)
{
    _tlsPoolTable[header->GetAllocSize()]->Push(header);
}
