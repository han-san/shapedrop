#pragma once

#include "jint.h"

#include <gsl/gsl>

#include <concepts>
#include <limits>
#include <optional>

template <std::unsigned_integral BackingType>
class Positive {
public:
  Positive() = default;

  template <std::integral FromIntType>
  constexpr explicit(std::is_signed_v<FromIntType>) Positive(const FromIntType i)
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
  constexpr explicit Positive(const Float i) : m_value {gsl::narrow_cast<BackingType>(i)} {
    // FIXME: Rounding error could cause an unintended exception.
    if (i < 0 or i > static_cast<Float>(std::numeric_limits<BackingType>::max())) {
      throw gsl::narrowing_error();
    }
  }

  template <std::floating_point Float>
  [[nodiscard]] static constexpr auto try_construct(Float i)
      -> std::optional<Positive<BackingType>> {
    // FIXME: Rounding error could cause an unintended failure.
    if (i < 0 or i > static_cast<Float>(std::numeric_limits<BackingType>::max())) {
      return std::nullopt;
    }
    return Positive<BackingType> {gsl::narrow_cast<BackingType>(i)};
  }

  template <std::integral FromType>
  [[nodiscard]] static constexpr auto try_construct(FromType from)
      -> std::optional<Positive<BackingType>> {
    // TODO: How should max value narrowing work? Currently, in the contructor it's checked and an
    // exception is throw, but here we don't allow it at compile time.
    constexpr auto fromTypeMax = std::numeric_limits<FromType>::max();
    static_assert(fromTypeMax >= 0 and fromTypeMax <= std::numeric_limits<BackingType>::max());

    if constexpr (std::is_unsigned_v<FromType>) {
      return Positive<BackingType> {from};
    }
    if (from < 0) {
      return std::nullopt;
    }
    return Positive<BackingType> {gsl::narrow_cast<BackingType>(from)};
  }

  [[nodiscard]] constexpr explicit operator BackingType() const noexcept { return m_value; }

  template <typename U>
  constexpr operator Positive<U>() const noexcept {
    return m_value;
  }

  [[nodiscard]] constexpr explicit operator bool() const noexcept { return m_value; }

  constexpr auto operator-=(const Positive& rhs) -> Positive& {
    if (rhs > *this) {
      throw std::underflow_error("Subtraction of PositiveGeneric underflowed");
    }

    m_value -= BackingType {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator-(Positive lhs, const Positive& rhs) -> Positive {
    return lhs -= rhs;
  }
  constexpr auto operator+=(const Positive& rhs) -> Positive& {
    m_value += BackingType {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator+(Positive lhs, const Positive& rhs) -> Positive {
    return lhs += rhs;
  }
  constexpr auto operator*=(const Positive& rhs) -> Positive& {
    m_value *= BackingType {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator*(Positive lhs, const Positive& rhs) -> Positive {
    return lhs *= rhs;
  }
  constexpr auto operator/=(const Positive& rhs) -> Positive& {
    m_value /= BackingType {rhs};
    return *this;
  }
  [[nodiscard]] friend constexpr auto operator/(Positive lhs, const Positive& rhs) -> Positive {
    return lhs /= rhs;
  }

  friend constexpr auto operator<=>(const Positive& lhs, const Positive& rhs) = default;

private:
  BackingType m_value {};
};

using PositiveUChar = Positive<uchar>;
using PositiveUShort = Positive<ushort>;
using PositiveUInt = Positive<uint>;
using PositiveULong = Positive<ulong>;
using PositiveULLong = Positive<ullong>;
using PositiveU8 = Positive<u8>;
using PositiveU16 = Positive<u16>;
using PositiveU32 = Positive<u32>;
using PositiveU64 = Positive<u64>;
using PositiveSize_t = Positive<std::size_t>;

static_assert(PositiveU8::try_construct(u8 {2}) == std::optional<PositiveU8>(2));
static_assert(PositiveU8::try_construct(s8 {-1}) == std::nullopt);
static_assert(PositiveU8::try_construct(float {2}) == std::optional<PositiveU8>(2));
static_assert(PositiveU8::try_construct(double {-1}) == std::nullopt);

