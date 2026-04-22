#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(uint32 bufferSize) 
    : _bufferSize(bufferSize)
    , _capacity(bufferSize * BUFFER_COUNT)
    , _buffer(_capacity)
{}

bool RecvBuffer::OnWrite(uint32 numOfBytes)
{
	if (numOfBytes > GetFreeSize())
		return false;

	_writePos += numOfBytes;
	return true;
}

bool RecvBuffer::OnRead(uint32 numOfBytes)
{
	if (numOfBytes > GetDataSize())
		return false;

	_readPos += numOfBytes;
	return true;
}

void RecvBuffer::Clean()
{
    uint32 dataSize = GetDataSize();

    if (dataSize == 0)
    {
        _readPos = _writePos = 0;
        return;
    }

    if (GetFreeSize() < _bufferSize)
    {
        ::memmove(_buffer.data(), _buffer.data() + _readPos, dataSize);
        _readPos = 0;
        _writePos = dataSize;
    }
}
