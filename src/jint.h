#pragma once

#include <stdint.h>

#ifdef __cplusplus
#include <cstddef>
#endif

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

[[nodiscard]] auto constexpr operator "" _s8 (ullong n) -> s8  { return static_cast<s8> (n); }
[[nodiscard]] auto constexpr operator "" _s16(ullong n) -> s16 { return static_cast<s16>(n); }
[[nodiscard]] auto constexpr operator "" _s32(ullong n) -> s32 { return static_cast<s32>(n); }
[[nodiscard]] auto constexpr operator "" _s64(ullong n) -> s64 { return static_cast<s64>(n); }

[[nodiscard]] auto constexpr operator "" _u8 (ullong n) -> u8  { return static_cast<u8> (n); }
[[nodiscard]] auto constexpr operator "" _u16(ullong n) -> u16 { return static_cast<u16>(n); }
[[nodiscard]] auto constexpr operator "" _u32(ullong n) -> u32 { return static_cast<u32>(n); }
[[nodiscard]] auto constexpr operator "" _u64(ullong n) -> u64 { return static_cast<u64>(n); }

[[nodiscard]] auto constexpr operator "" _zu(ullong n) -> std::size_t { return static_cast<std::size_t>(n); }

#endif
