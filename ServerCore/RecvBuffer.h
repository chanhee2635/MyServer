#pragma once

enum class BufferMode { Linear, Circular };

class RecvBuffer
{
public:
	RecvBuffer(BufferMode mode = BufferMode::Circular, uint32 bufferSize = Config::Session::RECV_BUFFER_SIZE);

	uint32 GetWriteSegments(WSABUF wsaBufs[2]);

	std::span<const BYTE> ReadSegment() const;

	void Linearize();

	uint32 GetFreeSize() const;
	uint32 GetDataSize() const;
	bool   OnWrite(uint32 numOfBytes);
	bool   OnRead(uint32 numOfBytes);

private:
	void CleanLinear();

	static constexpr uint32 BUFFER_COUNT = Config::Session::RECV_BUFFER_COUNT;

	BufferMode   _mode;
	uint32		 _bufferSize = 0;
	uint32		 _capacity   = 0;
	uint32		 _readPos    = 0;
	uint32		 _writePos   = 0;
	uint32       _dataSize   = 0;
	Vector<BYTE> _buffer;
};

