#pragma once
#include <stdint.h>

static inline uint32_t _lrotl(uint32_t value, int shift)
{
#if defined(__has_builtin) && __has_builtin(__builtin_rotateleft32)
    return __builtin_rotateleft32(value, shift);
#else
    shift &= 31;
    return ((value << shift) | (value >> (32 - shift)));
#endif
}