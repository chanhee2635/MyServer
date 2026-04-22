#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize)
    : _owner(owner)
    , _buffer(buffer)
    , _allocSize(allocSize)
{}

void SendBuffer::Close(uint32 writeSize)
{
    ASSERT_CRASH(writeSize <= _allocSize);
    _writeSize = writeSize;
    _owner->Close(writeSize);
}

SendBufferChunk::SendBufferChunk()
{}

void SendBufferChunk::Reset()
{
    _usedSize = 0;
    _open = false;
}

SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
    ASSERT_CRASH(allocSize <= GetFreeSize());
    ASSERT_CRASH(_open == false);

    _open = true;
    return MakeShared<SendBuffer>(shared_from_this(), GetWritePos(), allocSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
    ASSERT_CRASH(_open == true);
    _usedSize += writeSize;
    _open = false;
}

SendBufferRef SendBufferManager::Open(uint32 allocSize)
{
    ASSERT_CRASH(allocSize <= Config::Buffer::SEND_BUFFER_CHUNK_SIZE);

    if (LSendBufferChunk == nullptr)
        LSendBufferChunk = Pop();

    ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);

    if (LSendBufferChunk->GetFreeSize() < allocSize)
        LSendBufferChunk = Pop();

    return LSendBufferChunk->Open(allocSize);
}

SendBufferChunkRef SendBufferManager::Pop()
{
    {
        std::lock_guard<std::mutex> guard(_lock);
        if (!_chunks.empty())
        {
            SendBufferChunkRef chunk = _chunks.back();
            _chunks.pop_back();
            return chunk;
        }
    }
    return SendBufferChunkRef(xnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::PushGlobal(SendBufferChunk* chunk)
{
    if (GSendBufferManager == nullptr)
    {
        xdelete(chunk);
        return;
    }

    chunk->Reset();
    GSendBufferManager->Push(SendBufferChunkRef(chunk, PushGlobal));
}

void SendBufferManager::Push(SendBufferChunkRef chunk)
{
    std::lock_guard<std::mutex> guard(_lock);
    _chunks.push_back(chunk);
}
