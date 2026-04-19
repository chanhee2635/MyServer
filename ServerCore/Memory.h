#pragma once
#include "Allocator.h"

//class MemoryManager {
//public:
//	MemoryManager();
//	~MemoryManager();
//
//	void Init();
//
//	void* Allocate(int32 size);
//	void Release(void* ptr);
//
//	void* AllocateFrame(int32 size);
//
//private:
//	static constexpr int32 SLAB_1_SIZE = 1024;
//	static constexpr int32 SLAB_2_SIZE = 2048;
//	static constexpr int32 MAX_ALLOC_SIZE = 4096;
//	static constexpr int32 POOL_COUNT = (SLAB_1_SIZE / 32) + (SLAB_1_SIZE / 128) + (SLAB_2_SIZE / 256);
//
//	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
//	std::vector<MemoryPool*> _pools;
//
//	FrameAllocator _frameAllocator;
//};

template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));

	new(memory)Type(forward<Args>(args)...);

	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	BaseAllocator::Release(obj);
}

template<typename Type, typename... Args>
std::shared_ptr<Type> MakeShared(Args&&... args)
{
	return std::shared_ptr<Type>{ xnew<Type>(forward<Args>(args)...), xdelete<Type> };
}