#pragma once

namespace Config 
{
	static constexpr uint32 KB = 1024;
	static constexpr uint32 MB = 1024 * KB;

	namespace Memory 
	{
		static constexpr uint32 DEFAULT_ALIGNMENT = 16;
		static constexpr uint32 SLIST_ALIGNMENT = 16;
		static constexpr uint32 ALLOC_COUNT = 32;
		static constexpr uint32 MAGIC_NUMBER = 0x12345678;

		static constexpr uint32 MAX_POOL_SIZE = 4096;
		static constexpr uint32 POOL_COUNT = (1024/32) + (1024/128) + (2048/256);

		static constexpr uint32 TLS_MAX_COUNT   = 32;
		static constexpr uint32 TLS_BATCH_COUNT = 16;

		namespace Frame
		{
			static constexpr uint32 DEFAULT_BUFFER_SIZE = 10 * MB;
		}
	}

	namespace Session
	{
		static constexpr uint32 DEFAULT_ACCEPT_COUNT = 10;
		static constexpr uint32 RECV_BUFFER_SIZE = 4 * KB;
		static constexpr uint32 RECV_BUFFER_COUNT = 10;
	}

	namespace Buffer
	{
		static constexpr uint32 SEND_BUFFER_CHUNK_SIZE = 4 * KB;
	}

	namespace Network
	{
		static constexpr uint32 ADDR_BUFFER_SIZE = 64;
		static constexpr uint32 ADDR_BUFFER_ALIGNMENT = 16;
	}
}