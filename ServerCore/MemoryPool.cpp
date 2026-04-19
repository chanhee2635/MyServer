#include "pch.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize)
{
	::InitializeSListHead(&_header);
}

MemoryPool::~MemoryPool()
{
	while (MemoryHeader* header = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header)))
		::_aligned_free(header);
}

void MemoryPool::Push(void* ptr)
{
    MemoryHeader* header = MemoryHeader::DetachHeader(ptr);
    ASSERT_CRASH(header->GetMagic() == MagicNumber);

    ::InterlockedPushEntrySList(&_header, static_cast<PSLIST_ENTRY>(header));

    _useCount.fetch_sub(1);
    _reserveCount.fetch_add(1);
}

void* MemoryPool::Pop()
{
    MemoryHeader* header = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header));

    if (header == nullptr)
    {
        for (int32 i = 0; i < AllocCount - 1; ++i)
        {
            MemoryHeader* newHeader = static_cast<MemoryHeader*>(::_aligned_malloc(_allocSize, AlignSize));
            ::InterlockedPushEntrySList(&_header, static_cast<PSLIST_ENTRY>(newHeader));
        }

        _reserveCount.fetch_add(AllocCount - 1);
        header = static_cast<MemoryHeader*>(::_aligned_malloc(_allocSize, AlignSize));
    }
    else
    {
        _reserveCount.fetch_sub(1);
    }

    _useCount.fetch_add(1);
    return MemoryHeader::AttachHeader(header, _allocSize);
}
