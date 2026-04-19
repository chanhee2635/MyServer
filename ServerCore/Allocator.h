#pragma once
// Policy
class IAllocator
{
public:
	virtual ~IAllocator() {}
	virtual void* Allocate(int32 size) = 0;
	virtual void  Release(void* ptr) = 0;
};

// Global, OverMaxSize 
class BaseAllocator : public IAllocator
{
public:
	virtual void* Allocate(int32 size) override;
	virtual void  Release(void* ptr) override;
};

// Temporary(TLS)
class FrameAllocator : public IAllocator
{
public:
	FrameAllocator(int32 bufferSize = 10 * 1024 * 1024);
	virtual ~FrameAllocator() override;

	virtual void* Allocate(int32 size) override;
	virtual void  Release(void* ptr) override;

	void Clear();

private:
	uint8* _buffer = nullptr;
	uint8* _freePtr = nullptr;
	uint8* _endPtr = nullptr;
	int32  _bufferSize = 0;
};

// Use-after-free, Buffer Overflow
class StompAllocator : public IAllocator
{
public:
	virtual void* Allocate(int32 size) override;
	virtual void  Release(void* ptr) override;

private:
	static const int32 GPageSize;
};

template<typename T, AllocType Type = AllocType::Pool>
class StlAllocator {
public:
	using value_type = T;

	StlAllocator() noexcept {}
	template<typename Other> StlAllocator(const StlAllocator<Other, Type>&) noexcept {}

	T* allocate(size_t count) {
		const int32 size = static_cast<int32>(count * sizeof(T));

		/*if constexpr (Type == AllocType::Frame)
			return static_cast<T*>(GMemory.AllocateFrame(size));
		else
			return static_cast<T*>(GMemory.Allocate(size));*/

		return nullptr;
	}

	void deallocate(T* ptr, size_t count)
	{
		xrelease(ptr);
	}
};