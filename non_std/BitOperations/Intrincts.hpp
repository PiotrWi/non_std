#pragma once

#ifdef _MSC_VER
    #include <intrin.h>
#endif // _MSC_VER

namespace bit_operations::intrincs
{

    // Returns one plus the index of the least significant 1-bit of x, or if x is zero, returns zero.
    // Eg: 0x0010 return 2
    constexpr unsigned char findFirstSet(uint64_t in)
    {
#ifdef __GNUC__
        return __builtin_ffsll(in);
#endif // __GNUC__
#ifdef _MSC_VER 
        return _tzcnt_u64(in) + 1;
#endif // _MSC_VER
        static_assert ("Not supported compiler");
        return 0;
    }
}