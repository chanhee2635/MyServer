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

		namespace Frame
		{
			static constexpr uint32 DEFAULT_BUFFER_SIZE = 10 * MB;
		}
	}
}