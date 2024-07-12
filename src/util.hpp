#pragma once

#include "jint.h"
#include "positive.hpp"

#include <array>
#include <cassert>
#include <iterator>

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
