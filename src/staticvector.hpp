#pragma once

#include <array>
#include <cassert>
#include <iterator>

// TODO: Replace with std::inplace_vector when it gets implemented in compilers.
template <typename T, std::size_t maxSize>
class StaticVector {
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
  [[nodiscard]] constexpr auto end() noexcept -> iterator {
    return std::next(begin(), static_cast<difference_type>(m_size));
  }
  [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
    return std::next(begin(), static_cast<difference_type>(m_size));
  }
  [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator { return m_data.cbegin(); }
  [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator {
    return std::next(cbegin(), static_cast<difference_type>(m_size));
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
