#pragma once

#include "jint.h"

#include <gsl/gsl>

#include <array>
#include <cassert>
#include <concepts>
#include <limits>
#include <stdexcept>

template <std::unsigned_integral T>
class PositiveGeneric {
  using ThisType = PositiveGeneric<T>;

public:
  PositiveGeneric() = default;

  template <std::integral FromIntType>
  constexpr explicit(std::is_signed_v<FromIntType>) PositiveGeneric(const FromIntType i)
      : m_value {gsl::narrow_cast<T>(i)} {
    static_assert(std::numeric_limits<T>::min() == 0);
    constexpr auto lowerBoundIsUnsafe = std::numeric_limits<FromIntType>::min() < 0;
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
  constexpr explicit PositiveGeneric(const Float i) : m_value {gsl::narrow_cast<T>(i)} {
    // FIXME: Rounding error could cause an unintended exception.
    if (i < 0 || i > static_cast<Float>(std::numeric_limits<T>::max())) {
      throw gsl::narrowing_error();
    }
  }

  [[nodiscard]] constexpr explicit operator T() const noexcept { return m_value; }

  template <typename U>
  operator PositiveGeneric<U>() const noexcept {
    return m_value;
  }

  [[nodiscard]] explicit operator bool() const noexcept { return m_value; }

  constexpr auto operator-=(const ThisType& rhs) -> ThisType& {
    if (rhs > *this) {
      throw std::underflow_error("Subtraction of PositiveGeneric underflowed");
    }

    m_value -= T {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator-(ThisType lhs, const ThisType& rhs) -> ThisType {
    return lhs -= rhs;
  }
  constexpr auto operator+=(const ThisType& rhs) -> ThisType& {
    m_value += T {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator+(ThisType lhs, const ThisType& rhs) -> ThisType {
    return lhs += rhs;
  }
  constexpr auto operator*=(const ThisType& rhs) -> ThisType& {
    m_value *= T {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator*(ThisType lhs, const ThisType& rhs) -> ThisType {
    return lhs *= rhs;
  }
  constexpr auto operator/=(const ThisType& rhs) -> ThisType& {
    m_value /= T {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator/(ThisType lhs, const ThisType& rhs) -> ThisType {
    return lhs /= rhs;
  }

  friend constexpr auto operator<=>(const ThisType& lhs, const ThisType& rhs) = default;

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

  constexpr auto operator+=(const ThisType& rhs) -> ThisType& {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator+(ThisType lhs, const ThisType& rhs) -> ThisType {
    return lhs += rhs;
  }

  [[nodiscard]] static constexpr auto right() -> ThisType { return {1, 0}; }
  [[nodiscard]] static constexpr auto left() -> ThisType { return {-1, 0}; }
  [[nodiscard]] static constexpr auto up() -> ThisType { return {0, -1}; }
  [[nodiscard]] static constexpr auto down() -> ThisType { return {0, 1}; }
};

using V2 = V2Generic<int>;
using V2f = V2Generic<double>;

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

  constexpr auto operator*=(const T& rhs) {
    x *= rhs;
    y *= rhs;
    w *= rhs;
    h *= rhs;
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator*(Rect<T> lhs, const T& rhs) { return lhs *= rhs; }
};

template <typename T>
struct Point {
  T x {};
  T y {};

  constexpr auto operator+=(const V2Generic<T>& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator+(Point<T> lhs, const V2Generic<T>& rhs) {
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

template <typename T>
[[nodiscard]] auto point_is_in_rect(const Point<T>& point, const Rect<T>& rect) {
  return (point.x >= rect.x) and (point.x < rect.x + rect.w) and (point.y >= rect.y) and
         (point.y < rect.y + rect.h);
}

// This won't work if T doesn't have a default constructor. If it does have
// one, there's a potential performance hit from default constructing every
// element and then immediately reassigning all of them.
template <typename T, std::size_t N>
[[nodiscard]] constexpr auto make_filled_array(const T& val) {
  std::array<T, N> arr;
  arr.fill(val);
  return arr;
}
