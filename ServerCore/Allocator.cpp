#include "pch.h"
#include "Allocator.h"

void* BaseAllocator::Allocate(uint32 size)
{
    return ::_aligned_malloc(size, AlignSize);
}

void BaseAllocator::Release(void* ptr)
{
    ::_aligned_free(ptr);
}

const int32 StompAllocator::GPageSize = []() {
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);
    return static_cast<int32>(info.dwPageSize);
}();

void* StompAllocator::Allocate(uint32 size)
{
    const uint32 alignedSize = MemoryUtils::AlignUp(size, AlignSize);

    const uint32 dataPageCount = (alignedSize + GPageSize - 1) / GPageSize;
    const uint32 totalPageCount = dataPageCount + 1; 

    void* baseAddress = ::VirtualAlloc(NULL, static_cast<size_t>(totalPageCount * GPageSize), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    void* guardPage = static_cast<int8*>(baseAddress) + (dataPageCount * GPageSize);
    DWORD oldProtect;
    ::VirtualProtect(guardPage, GPageSize, PAGE_NOACCESS, &oldProtect);

    const uint32 dataOffset = (dataPageCount * GPageSize) - alignedSize;
    return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset);
}

void StompAllocator::Release(void* ptr)
{
    if (ptr == nullptr) return;

    const int64 address = reinterpret_cast<int64>(ptr);
    const int64 baseAddress = address - (address % GPageSize);
    ::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}

FrameAllocator::FrameAllocator(uint32 bufferSize) : _bufferSize(bufferSize)
{
    _buffer = static_cast<uint8*>(::_aligned_malloc(bufferSize, AlignSize));
    _freePtr = _buffer;
    _endPtr = _buffer + bufferSize;
}

FrameAllocator::~FrameAllocator()
{
    if (_buffer)
    {
        ::_aligned_free(_buffer);
        _buffer = nullptr;
    }
}

void* FrameAllocator::Allocate(uint32 size)
{
    const uint32 alignedSize = MemoryUtils::AlignUp(size, AlignSize);

    if (_freePtr + alignedSize > _endPtr)
    {
        // Size up Or Base Allocator
        std::cout << "Frame Overflow! Size: " << size << " Falling back to Base." << std::endl;
        ASSERT_CRASH(false);
    }

    void* ptr = _freePtr;
    _freePtr += alignedSize;

    return ptr;
}

void FrameAllocator::Release(void* ptr)
{
}

void FrameAllocator::Clear()
{
    _freePtr = _buffer;
}
