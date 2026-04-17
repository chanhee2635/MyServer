#pragma once
// Policy
class IAllocator
{
public:
	virtual ~IAllocator() {}
	virtual void* Allocate(int32 size) = 0;
	virtual void  Release(void* ptr) = 0;
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