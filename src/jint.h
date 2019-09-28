#pragma once

#include <stdint.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef long long llong;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long ullong;

typedef signed char schar;
typedef signed short sshort;
typedef signed int sint;
typedef signed long slong;
typedef signed long long sllong;

#ifdef __cplusplus

s8  constexpr operator "" _s8 (ullong n) { return (s8 ) n; }
s16 constexpr operator "" _s16(ullong n) { return (s16) n; }
s32 constexpr operator "" _s32(ullong n) { return (s32) n; }
s64 constexpr operator "" _s64(ullong n) { return (s64) n; }

u8  constexpr operator "" _u8 (ullong n) { return (u8 ) n; }
u16 constexpr operator "" _u16(ullong n) { return (u16) n; }
u32 constexpr operator "" _u32(ullong n) { return (u32) n; }
u64 constexpr operator "" _u64(ullong n) { return (u64) n; }

size_t constexpr operator "" _usize(ullong n) { return (size_t) n; }

#endif
