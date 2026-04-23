#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(BufferMode mode, uint32 bufferSize)
    : _mode(mode)
    , _bufferSize(bufferSize)
    , _capacity(mode == BufferMode::Linear ? bufferSize * BUFFER_COUNT : bufferSize)
    , _buffer(_capacity)
{}

uint32 RecvBuffer::GetFreeSize() const
{
    switch (_mode)
    {
    case BufferMode::Linear:   return _capacity - _writePos;
    case BufferMode::Circular: return _capacity - _dataSize;
    }
    return 0;
}

uint32 RecvBuffer::GetDataSize() const
{
    switch (_mode)
    {
    case BufferMode::Linear:   return _writePos - _readPos;
    case BufferMode::Circular: return _dataSize;
    }
    return 0;
}

uint32 RecvBuffer::GetWriteSegments(WSABUF wsaBufs[2])
{
    uint32 freeSize = GetFreeSize();
    if (freeSize == 0)
        return 0;

    switch (_mode)
    {
    case BufferMode::Linear:
    {
        wsaBufs[0].buf = reinterpret_cast<char*>(_buffer.data() + _writePos);
        wsaBufs[0].len = static_cast<ULONG>(freeSize);
        return 1;
    }
    case BufferMode::Circular:
    {
        uint32 tailSize = _capacity - _writePos; // writePos 부터 끝까지

        if (tailSize >= freeSize)
        {
            // wrap 없이 끝까지 쓸 수 있음
            wsaBufs[0].buf = reinterpret_cast<char*>(_buffer.data() + _writePos);
            wsaBufs[0].len = static_cast<ULONG>(freeSize);
            return 1;
        }
        else
        {
            // wrap: 끝까지 + 처음부터 남은 공간
            wsaBufs[0].buf = reinterpret_cast<char*>(_buffer.data() + _writePos);
            wsaBufs[0].len = static_cast<ULONG>(tailSize);
            wsaBufs[1].buf = reinterpret_cast<char*>(_buffer.data());
            wsaBufs[1].len = static_cast<ULONG>(freeSize - tailSize);
            return 2;
        }
    }
    }
    return 0;
}

std::span<const BYTE> RecvBuffer::ReadSegment() const
{
    switch (_mode)
    {
    case BufferMode::Linear:
        return { _buffer.data() + _readPos, GetDataSize() };

    case BufferMode::Circular:
    {
        // wrap 에 걸린 경우 끝까지만 반환 → 호출자가 Linearize() 후 재호출
        uint32 tailSize = _capacity - _readPos;
        uint32 contiguous = (_dataSize < tailSize) ? _dataSize : tailSize;
        return { _buffer.data() + _readPos, contiguous };
    }
    }
    return {};
}

void RecvBuffer::Linearize()
{
    if (_mode != BufferMode::Circular)
        return;

    if (_readPos + _dataSize <= _capacity)
        return;

    ServerStats::Get().recvBuffer.linearizeCount++;

    Vector<BYTE> temp(_dataSize);
    uint32 tailSize = _capacity - _readPos;

    ::memcpy(temp.data(), _buffer.data() + _readPos, tailSize);
    ::memcpy(temp.data() + tailSize, _buffer.data(), _dataSize - tailSize);
    ::memcpy(_buffer.data(), temp.data(), _dataSize);

    _readPos = 0;
    _writePos = _dataSize;
}

bool RecvBuffer::OnWrite(uint32 numOfBytes)
{
    if (numOfBytes > GetFreeSize())
        return false;

    switch (_mode)
    {
    case BufferMode::Linear:
        _writePos += numOfBytes;
        break;
    case BufferMode::Circular:
        _writePos = (_writePos + numOfBytes) % _capacity;
        _dataSize += numOfBytes;
        break;
    }
    return true;
}

bool RecvBuffer::OnRead(uint32 numOfBytes)
{
    if (numOfBytes > GetDataSize())
        return false;

    switch (_mode)
    {
    case BufferMode::Linear:
        _readPos += numOfBytes;
        CleanLinear();
        break;
    case BufferMode::Circular:
        _readPos = (_readPos + numOfBytes) % _capacity;
        _dataSize -= numOfBytes;
        // 데이터 소진 시 위치 초기화 (locality 향상)
        if (_dataSize == 0)
            _readPos = _writePos = 0;
        break;
    }
    return true;
}

void RecvBuffer::CleanLinear()
{
    uint32 dataSize = GetDataSize();

    if (dataSize == 0)
    {
        _readPos = _writePos = 0;
        return;
    }

    if (GetFreeSize() < _bufferSize)
    {
        ServerStats::Get().recvBuffer.memmoveCount++;
        ::memmove(_buffer.data(), _buffer.data() + _readPos, dataSize);
        _readPos = 0;
        _writePos = dataSize;
    }
}