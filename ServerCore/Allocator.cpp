#include "pch.h"
#include "Allocator.h"

const int32 StompAllocator::GPageSize = []() {
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);
    return static_cast<int32>(info.dwPageSize);
}();

void* StompAllocator::Allocate(int32 size)
{
    const int32 alignedSize = (size + 15) & ~15;

    const int32 dataPageCount = (alignedSize + GPageSize - 1) / GPageSize;
    const int32 totalPageCount = dataPageCount + 1; 

    void* baseAddress = ::VirtualAlloc(NULL, static_cast<size_t>(totalPageCount * GPageSize), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    void* guardPage = static_cast<int8*>(baseAddress) + (dataPageCount * GPageSize);
    DWORD oldProtect;
    ::VirtualProtect(guardPage, GPageSize, PAGE_NOACCESS, &oldProtect);

    const int32 dataOffset = (dataPageCount * GPageSize) - alignedSize;
    return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset);
}

void StompAllocator::Release(void* ptr)
{
    if (ptr == nullptr) return;

    const int64 address = reinterpret_cast<int64>(ptr);
    const int64 baseAddress = address - (address % GPageSize);
    ::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}
