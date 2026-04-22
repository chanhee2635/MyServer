#pragma once

class RecvBuffer
{
public:
	RecvBuffer(uint32 bufferSize = Config::Session::RECV_BUFFER_SIZE);

	std::span<BYTE>		  WriteSegment() { return { _buffer.data() + _writePos, GetFreeSize() }; }
	std::span<const BYTE> ReadSegment()  const { return { _buffer.data() + _readPos,  GetDataSize() }; }

	uint32 GetFreeSize() const { return _capacity - _writePos; }
	uint32 GetDataSize() const { return _writePos - _readPos; }

	bool OnWrite(uint32 numOfBytes);
	bool OnRead(uint32 numOfBytes);
	void Clean();

private:
	static constexpr uint32 BUFFER_COUNT = Config::Session::RECV_BUFFER_COUNT;

	uint32		 _bufferSize = 0;
	uint32		 _capacity = 0;
	uint32		 _writePos = 0;
	uint32		 _readPos = 0;
	Vector<BYTE> _buffer;
};

