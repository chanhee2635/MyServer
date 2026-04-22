#pragma once

class SendBufferChunk;

class SendBuffer
{
public:
    SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize);

    BYTE*  GetBuffer() { return _buffer; }
    uint32 GetWriteSize() const { return _writeSize; }
    void   Close(uint32 writeSize);

private:
    SendBufferChunkRef _owner;
    BYTE* _buffer = nullptr;
    uint32 _allocSize = 0;
    uint32 _writeSize = 0;
};

class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk>
{
    friend class SendBuffer;

public:
    SendBufferChunk();

    void          Reset();
    SendBufferRef Open(uint32 allocSize);
    bool          IsOpen()    const { return _open; }
    uint32        GetFreeSize() const { return CHUNK_SIZE - _usedSize; }

private:
    void  Close(uint32 writeSize);
    BYTE* GetWritePos() { return _buffer + _usedSize; }

    static constexpr uint32 CHUNK_SIZE = Config::Buffer::SEND_BUFFER_CHUNK_SIZE;

    BYTE   _buffer[CHUNK_SIZE] = {};
    uint32 _usedSize = 0;
    bool   _open = false;
};

class SendBufferManager
{
public:
    SendBufferRef Open(uint32 allocSize);
    void          Push(SendBufferChunkRef chunk);

private:
    SendBufferChunkRef Pop();
    static void        PushGlobal(SendBufferChunk* chunk);

    std::mutex                  _lock;
    Vector<SendBufferChunkRef>  _chunks;
};