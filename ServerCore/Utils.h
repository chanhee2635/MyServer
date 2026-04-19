#pragma once

namespace MemoryUtils
{
    template<typename T>
    constexpr T AlignUp(T size, T alignment)
    {
        return (size + (alignment - 1)) & ~(alignment - 1);
    }
}