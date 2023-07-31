#pragma once

#include "jint.h"

#include <gsl/gsl>

#include <array>
#include <cassert>
#include <concepts>
#include <limits>

template <std::unsigned_integral T>
class PositiveGeneric {
  using ThisType = PositiveGeneric<T>;

public:
  PositiveGeneric() = default;

  template <std::integral FromIntType>
  constexpr explicit(std::is_signed_v<FromIntType>)
      PositiveGeneric(const FromIntType i)
      : m_value {gsl::narrow_cast<T>(i)} {
    static_assert(std::numeric_limits<T>::min() == 0);
    constexpr auto lowerBoundIsUnsafe =
        std::numeric_limits<FromIntType>::min() < 0;
    if constexpr (lowerBoundIsUnsafe) {
      if (i < 0) {
        throw gsl::narrowing_error();
      }
    }

    constexpr auto upperBoundIsUnsafe =
        std::numeric_limits<FromIntType>::max() > std::numeric_limits<T>::max();
    if constexpr (upperBoundIsUnsafe) {
      if (std::numeric_limits<T>::max() < i) {
        throw gsl::narrowing_error();
      }
    }
  }

  template <std::floating_point Float>
  constexpr explicit PositiveGeneric(const Float i)
      : m_value {gsl::narrow_cast<T>(i)} {
    // make sure it fits the type's range
    assert(i >= 0);
    assert(i <= static_cast<Float>(std::numeric_limits<T>::max()));
  }

  [[nodiscard]] constexpr explicit operator T() const noexcept {
    return m_value;
  }

  template <typename U>
  operator PositiveGeneric<U>() const noexcept {
    return m_value;
  }

  [[nodiscard]] explicit operator bool() const noexcept { return m_value; }

  auto constexpr operator-=(ThisType const& rhs) -> ThisType& {
    auto const rhsVal = T {rhs};
    assert(m_value >= rhsVal);
    m_value -= rhsVal;
    return *this;
  }
  [[nodiscard]] auto constexpr friend operator-(ThisType lhs,
                                                ThisType const& rhs)
      -> ThisType {
    return lhs -= rhs;
  }
  auto constexpr operator+=(ThisType const& rhs) -> ThisType& {
    auto const rhsVal = T {rhs};
    m_value += rhsVal;
    return *this;
  }
  [[nodiscard]] auto constexpr friend operator+(ThisType lhs,
                                                ThisType const& rhs)
      -> ThisType {
    return lhs += rhs;
  }
  auto constexpr operator*=(ThisType const& rhs) -> ThisType& {
    auto const rhsVal = T {rhs};
    m_value *= rhsVal;
    return *this;
  }
  [[nodiscard]] auto constexpr friend operator*(ThisType lhs,
                                                ThisType const& rhs)
      -> ThisType {
    return lhs *= rhs;
  }
  auto constexpr operator/=(ThisType const& rhs) -> ThisType& {
    auto const rhsVal = T {rhs};
    m_value /= rhsVal;
    return *this;
  }
  [[nodiscard]] auto constexpr friend operator/(ThisType lhs,
                                                ThisType const& rhs)
      -> ThisType {
    return lhs /= rhs;
  }

  friend constexpr auto operator<=>(const ThisType& lhs,
                                    const ThisType& rhs) = default;

private:
  T m_value {};
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

template <typename T>
struct V2Generic {
  T x {};
  T y {};

  using ThisType = V2Generic<T>;

  auto constexpr operator+=(ThisType const& rhs) -> ThisType& {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  [[nodiscard]] auto constexpr friend operator+(ThisType lhs,
                                                ThisType const& rhs)
      -> ThisType {
    return lhs += rhs;
  }

  [[nodiscard]] auto static constexpr right() -> ThisType { return {1, 0}; }
  [[nodiscard]] auto static constexpr left() -> ThisType { return {-1, 0}; }
  [[nodiscard]] auto static constexpr up() -> ThisType { return {0, -1}; }
  [[nodiscard]] auto static constexpr down() -> ThisType { return {0, 1}; }
};

using V2 = V2Generic<int>;
using V2f = V2Generic<double>;

namespace Color {
struct RGBA {
  u8 static constexpr maxChannelValue {0xFF};
  struct Alpha {
    u8 static constexpr opaque {maxChannelValue};
    u8 static constexpr transparent {0};
  };

  PositiveU8 r {0U};
  PositiveU8 g {0U};
  PositiveU8 b {0U};
  PositiveU8 a {Alpha::opaque};
};

RGBA static constexpr red {RGBA::maxChannelValue, 0U, 0U};
RGBA static constexpr green {0U, RGBA::maxChannelValue, 0U};
RGBA static constexpr blue {0U, 0U, RGBA::maxChannelValue};
RGBA static constexpr cyan {0U, RGBA::maxChannelValue, RGBA::maxChannelValue};
RGBA static constexpr white {RGBA::maxChannelValue, RGBA::maxChannelValue,
                             RGBA::maxChannelValue};
RGBA static constexpr black {0U, 0U, 0U};
RGBA static constexpr transparent {0U, 0U, 0U, RGBA::Alpha::transparent};

// An invalid color to give some visual feedback when a color hasn't been
// properly initialized. White isn't really used otherwise in the game, so
// hopefully it will be obvious that something is wrong.
RGBA static constexpr invalid {white};

struct Shape {
  RGBA static constexpr I {0U, 0xF0U, 0xF0U};
  RGBA static constexpr O {0xF0U, 0xF0U, 0U};
  RGBA static constexpr L {0xF0U, 0xA0U, 0U};
  RGBA static constexpr J {0U, 0U, 0xF0U};
  RGBA static constexpr S {0U, 0xF0U, 0U};
  RGBA static constexpr Z {0xF0U, 0U, 0U};
  RGBA static constexpr T {0xA0U, 0U, 0xF0U};
};
} // namespace Color

template <typename T>
struct Rect {
  T x {};
  T y {};
  T w {};
  T h {};

  struct Size {
    T w {};
    T h {};
  };

  auto constexpr operator*=(T const& rhs) {
    x *= rhs;
    y *= rhs;
    w *= rhs;
    h *= rhs;
    return *this;
  }
  [[nodiscard]] auto constexpr friend operator*(Rect<T> lhs, T const& rhs) {
    return lhs *= rhs;
  }
};

template <typename T>
struct Point {
  T x {};
  T y {};

  auto constexpr operator+=(V2Generic<T> const& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  [[nodiscard]] auto constexpr friend operator+(Point<T> lhs,
                                                V2Generic<T> const& rhs) {
    return lhs += rhs;
  }
};

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

  [[nodiscard]] auto constexpr begin() noexcept -> iterator {
    return m_data.begin();
  }
  [[nodiscard]] auto constexpr begin() const noexcept -> const_iterator {
    return m_data.begin();
  }
  [[nodiscard]] auto constexpr end() noexcept -> iterator {
    return std::next(begin(), m_size);
  }
  [[nodiscard]] auto constexpr end() const noexcept -> const_iterator {
    return std::next(begin(), m_size);
  }
  [[nodiscard]] auto constexpr cbegin() const noexcept -> const_iterator {
    return m_data.cbegin();
  }
  [[nodiscard]] auto constexpr cend() const noexcept -> const_iterator {
    return std::next(cbegin(), m_size);
  }
  [[nodiscard]] auto constexpr front() -> reference { return m_data.front(); }
  [[nodiscard]] auto constexpr front() const -> const_reference {
    return m_data.front();
  }
  [[nodiscard]] auto constexpr back() -> reference {
    return m_data[m_size - 1];
  }
  [[nodiscard]] auto constexpr back() const -> const_reference {
    return m_data[m_size - 1];
  }
  [[nodiscard]] auto constexpr size() const noexcept -> size_type {
    return m_size;
  }
  [[nodiscard]] auto constexpr max_size() const noexcept -> size_type {
    return maxSize;
  }
  [[nodiscard]] auto constexpr empty() const noexcept -> bool {
    return m_size == 0;
  }
  auto constexpr push_back(const_reference i) -> void {
    m_data.at(m_size++) = i;
  }
  auto constexpr push_back(value_type&& i) -> void {
    m_data.at(m_size++) = std::move(i);
  }
  auto constexpr pop_back() -> void {
    assert(m_size > 0);
    m_data[--m_size].~T();
  }

private:
  ArrayType m_data;
  size_type m_size {0};
};

template <typename T>
[[nodiscard]] auto point_is_in_rect(Point<T> const& point,
                                    Rect<T> const& rect) {
  return (point.x >= rect.x) and (point.x < rect.x + rect.w) and
         (point.y >= rect.y) and (point.y < rect.y + rect.h);
}

// This won't work if T doesn't have a default constructor. If it does have
// one, there's a potential performance hit from default constructing every
// element and then immediately reassigning all of them.
template <typename T, std::size_t N>
[[nodiscard]] auto constexpr make_filled_array(T const& val) {
  std::array<T, N> arr;
  arr.fill(val);
  return arr;
}
