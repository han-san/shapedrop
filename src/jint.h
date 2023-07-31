#pragma once

#include <cstddef>
#include <cstdint>

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using llong = long long;

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;
using ullong = unsigned long long;

using schar = signed char;
using sshort = signed short;
using sint = signed int;
using slong = signed long;
using sllong = signed long long;

[[nodiscard]] constexpr auto operator"" _s8(ullong n) -> s8 { return static_cast<s8>(n); }
[[nodiscard]] constexpr auto operator"" _s16(ullong n) -> s16 { return static_cast<s16>(n); }
[[nodiscard]] constexpr auto operator"" _s32(ullong n) -> s32 { return static_cast<s32>(n); }
[[nodiscard]] constexpr auto operator"" _s64(ullong n) -> s64 { return static_cast<s64>(n); }

[[nodiscard]] constexpr auto operator"" _u8(ullong n) -> u8 { return static_cast<u8>(n); }
[[nodiscard]] constexpr auto operator"" _u16(ullong n) -> u16 { return static_cast<u16>(n); }
[[nodiscard]] constexpr auto operator"" _u32(ullong n) -> u32 { return static_cast<u32>(n); }
[[nodiscard]] constexpr auto operator"" _u64(ullong n) -> u64 { return static_cast<u64>(n); }

[[nodiscard]] constexpr auto operator"" _zu(ullong n) -> std::size_t {
  return static_cast<std::size_t>(n);
}