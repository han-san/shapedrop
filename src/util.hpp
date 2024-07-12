#pragma once

#include "jint.h"

#include <gsl/gsl>

#include <array>
#include <cassert>
#include <concepts>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>

template <std::unsigned_integral BackingType>
class PositiveGeneric {
  using ThisType = PositiveGeneric<BackingType>;

public:
  PositiveGeneric() = default;

  template <std::integral FromIntType>
  constexpr explicit(std::is_signed_v<FromIntType>) PositiveGeneric(const FromIntType i)
      : m_value {gsl::narrow_cast<BackingType>(i)} {
    static_assert(std::numeric_limits<BackingType>::min() == 0);
    constexpr auto lowerBoundIsUnsafe = std::numeric_limits<FromIntType>::min() < 0;
    if constexpr (lowerBoundIsUnsafe) {
      if (i < 0) {
        throw gsl::narrowing_error();
      }
    }

    constexpr auto upperBoundIsUnsafe =
        std::numeric_limits<FromIntType>::max() > std::numeric_limits<BackingType>::max();
    if constexpr (upperBoundIsUnsafe) {
      if (std::numeric_limits<BackingType>::max() < i) {
        throw gsl::narrowing_error();
      }
    }
  }

  template <std::floating_point Float>
  constexpr explicit PositiveGeneric(const Float i) : m_value {gsl::narrow_cast<BackingType>(i)} {
    // FIXME: Rounding error could cause an unintended exception.
    if (i < 0 or i > static_cast<Float>(std::numeric_limits<BackingType>::max())) {
      throw gsl::narrowing_error();
    }
  }

  template <std::floating_point Float>
  [[nodiscard]] static constexpr auto try_construct(Float i)
      -> std::optional<PositiveGeneric<BackingType>> {
    // FIXME: Rounding error could cause an unintended failure.
    if (i < 0 or i > static_cast<Float>(std::numeric_limits<BackingType>::max())) {
      return std::nullopt;
    }
    return PositiveGeneric<BackingType> {gsl::narrow_cast<BackingType>(i)};
  }

  template <std::integral FromType>
  [[nodiscard]] static constexpr auto try_construct(FromType from)
      -> std::optional<PositiveGeneric<BackingType>> {
    // TODO: How should max value narrowing work? Currently, in the contructor it's checked and an
    // exception is throw, but here we don't allow it at compile time.
    constexpr auto fromTypeMax = std::numeric_limits<FromType>::max();
    static_assert(fromTypeMax >= 0 and fromTypeMax <= std::numeric_limits<BackingType>::max());

    if constexpr (std::is_unsigned_v<FromType>) {
      return PositiveGeneric<BackingType> {from};
    }
    if (from < 0) {
      return std::nullopt;
    }
    return PositiveGeneric<BackingType> {gsl::narrow_cast<BackingType>(from)};
  }

  [[nodiscard]] constexpr explicit operator BackingType() const noexcept { return m_value; }

  template <typename U>
  operator PositiveGeneric<U>() const noexcept {
    return m_value;
  }

  [[nodiscard]] explicit operator bool() const noexcept { return m_value; }

  constexpr auto operator-=(const ThisType& rhs) -> ThisType& {
    if (rhs > *this) {
      throw std::underflow_error("Subtraction of PositiveGeneric underflowed");
    }

    m_value -= BackingType {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator-(ThisType lhs, const ThisType& rhs) -> ThisType {
    return lhs -= rhs;
  }
  constexpr auto operator+=(const ThisType& rhs) -> ThisType& {
    m_value += BackingType {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator+(ThisType lhs, const ThisType& rhs) -> ThisType {
    return lhs += rhs;
  }
  constexpr auto operator*=(const ThisType& rhs) -> ThisType& {
    m_value *= BackingType {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator*(ThisType lhs, const ThisType& rhs) -> ThisType {
    return lhs *= rhs;
  }
  constexpr auto operator/=(const ThisType& rhs) -> ThisType& {
    m_value /= BackingType {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator/(ThisType lhs, const ThisType& rhs) -> ThisType {
    return lhs /= rhs;
  }

  friend constexpr auto operator<=>(const ThisType& lhs, const ThisType& rhs) = default;

private:
  BackingType m_value {};
};

using PositiveUChar = PositiveGeneric<uchar>;
using PositiveUShort = PositiveGeneric<ushort>;
using PositiveUInt = PositiveGeneric<uint>;
using PositiveULong = PositiveGeneric<ulong>;
using PositiveULLong = PositiveGeneric<ullong>;
using PositiveU8 = PositiveGeneric<u8>;
using PositiveU16 = PositiveGeneric<u16>;
using PositiveU32 = PositiveGeneric<u32>;
using PositiveU64 = PositiveGeneric<u64>;
using PositiveSize_t = PositiveGeneric<std::size_t>;

static_assert(PositiveU8::try_construct(u8 {2}) == std::optional<PositiveU8>(2));
static_assert(PositiveU8::try_construct(s8 {-1}) == std::nullopt);
static_assert(PositiveU8::try_construct(float {2}) == std::optional<PositiveU8>(2));
static_assert(PositiveU8::try_construct(double {-1}) == std::nullopt);

namespace Color {
struct RGBA {
  static constexpr u8 maxChannelValue {0xFF};
  struct Alpha {
    static constexpr u8 opaque {maxChannelValue};
    static constexpr u8 transparent {0};
  };

  PositiveU8 r {0U};
  PositiveU8 g {0U};
  PositiveU8 b {0U};
  PositiveU8 a {Alpha::opaque};
};

static constexpr RGBA red {RGBA::maxChannelValue, 0U, 0U};
static constexpr RGBA green {0U, RGBA::maxChannelValue, 0U};
static constexpr RGBA blue {0U, 0U, RGBA::maxChannelValue};
static constexpr RGBA cyan {0U, RGBA::maxChannelValue, RGBA::maxChannelValue};
static constexpr RGBA white {RGBA::maxChannelValue, RGBA::maxChannelValue, RGBA::maxChannelValue};
static constexpr RGBA black {0U, 0U, 0U};
static constexpr RGBA transparent {0U, 0U, 0U, RGBA::Alpha::transparent};

// An invalid color to give some visual feedback when a color hasn't been
// properly initialized. White isn't really used otherwise in the game, so
// hopefully it will be obvious that something is wrong.
static constexpr RGBA invalid {white};

struct Shape {
  static constexpr RGBA I {0U, 0xF0U, 0xF0U};
  static constexpr RGBA O {0xF0U, 0xF0U, 0U};
  static constexpr RGBA L {0xF0U, 0xA0U, 0U};
  static constexpr RGBA J {0U, 0U, 0xF0U};
  static constexpr RGBA S {0U, 0xF0U, 0U};
  static constexpr RGBA Z {0xF0U, 0U, 0U};
  static constexpr RGBA T {0xA0U, 0U, 0xF0U};
};
} // namespace Color

template <typename T, std::size_t maxSize>
class ArrayStack {
  using ArrayType = std::array<T, maxSize>;

public:
  using value_type = typename ArrayType::value_type;
  using reference = typename ArrayType::reference;
  using const_reference = typename ArrayType::const_reference;
  using pointer = typename ArrayType::pointer;
  using const_pointer = typename ArrayType::const_pointer;
  using iterator = typename ArrayType::iterator;
  using const_iterator = typename ArrayType::const_iterator;
  using size_type = typename ArrayType::size_type;
  using difference_type = typename ArrayType::difference_type;

  [[nodiscard]] constexpr auto begin() noexcept -> iterator { return m_data.begin(); }
  [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator { return m_data.begin(); }
  [[nodiscard]] constexpr auto end() noexcept -> iterator { return std::next(begin(), m_size); }
  [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
    return std::next(begin(), m_size);
  }
  [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator { return m_data.cbegin(); }
  [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator {
    return std::next(cbegin(), m_size);
  }
  [[nodiscard]] constexpr auto front() -> reference { return m_data.front(); }
  [[nodiscard]] constexpr auto front() const -> const_reference { return m_data.front(); }
  [[nodiscard]] constexpr auto back() -> reference { return m_data[m_size - 1]; }
  [[nodiscard]] constexpr auto back() const -> const_reference { return m_data[m_size - 1]; }
  [[nodiscard]] constexpr auto size() const noexcept -> size_type { return m_size; }
  [[nodiscard]] constexpr auto max_size() const noexcept -> size_type { return maxSize; }
  [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_size == 0; }
  constexpr auto push_back(const_reference i) -> void { m_data.at(m_size++) = i; }
  constexpr auto push_back(value_type&& i) -> void { m_data.at(m_size++) = std::move(i); }
  constexpr auto pop_back() -> void {
    assert(m_size > 0);
    m_data[--m_size].~T();
  }

private:
  ArrayType m_data;
  size_type m_size {0};
};

// This won't work if T doesn't have a default constructor. If it does have
// one, there's a potential performance hit from default constructing every
// element and then immediately reassigning all of them.
template <typename T, std::size_t N>
[[nodiscard]] constexpr auto make_filled_array(const T& val) {
  std::array<T, N> arr;
  arr.fill(val);
  return arr;
}
