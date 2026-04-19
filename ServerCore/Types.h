#pragma once
#include <cstdint>

using int8   = std::int8_t;
using int16  = std::int16_t;
using int32  = std::int32_t;
using int64  = std::int64_t;
using uint8  = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using BYTE   = uint8;

using wstring = std::wstring;
using string = std::string;

// shared_ptr
using IocpObjectRef = std::shared_ptr<class IocpObject>;
using SessionRef = std::shared_ptr<class Session>;

#define size16(val)		static_cast<int16>(sizeof(val))
#define size32(val)		static_cast<int32>(sizeof(val))
template<typename T, size_t N>
constexpr int16 len16(T(&arr)[N]) { return static_cast<int16>(N); }
template<typename T, size_t N>
constexpr int32 len32(T(&arr)[N]) { return static_cast<int32>(N); }

enum class AllocType { Pool, Frame };
enum class ThreadType : uint8 { NONE, LOGIC, IO, DB };

