#pragma once

struct alignas(Config::Memory::SLIST_ALIGNMENT) MemoryHeader : public SLIST_ENTRY
{
	MemoryHeader(uint32 size) : _allocSize(size), _magic(Config::Memory::MAGIC_NUMBER) {}

	static void* AttachHeader(MemoryHeader* header, uint32 size)
	{
		new(header) MemoryHeader(size);
		return reinterpret_cast<void*>(header + 1);
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		return static_cast<MemoryHeader*>(ptr) - 1;
	}

	uint32 GetMagic() const { return _magic; }

private:
	uint32 _allocSize;
	uint32 _magic;
};

class  MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void  Push(void* ptr);
	void* Pop();

private:
	alignas(Config::Memory::SLIST_ALIGNMENT) SLIST_HEADER _header;
	int32			   _allocSize = 0;
	std::atomic<int32> _useCount = 0;
	std::atomic<int32> _reserveCount = 0;

	static constexpr uint32 AlignSize = Config::Memory::SLIST_ALIGNMENT;
	static constexpr uint32 MagicNumber = Config::Memory::MAGIC_NUMBER;
	static constexpr uint32 AllocCount = Config::Memory::ALLOC_COUNT;
};

