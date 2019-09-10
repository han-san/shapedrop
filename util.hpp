#pragma once

#include <array>

struct V2 {
    union {
        int w = 0;
        int x;
    };
    union {
        int h = 0;
        int y;
    };

    V2(int a = 0, int b = 0): x{a}, y{b} {}
};

struct V3 {
    union {
        int x;
        int r;
    };
    union {
        int y;
        int g;
    };
    union {
        int z;
        int b;
    };

    V3(int a = 0, int b = 0, int c = 0): x{a}, y{b}, z{c} {}
};

using Position = V2;
using Color = V3;

template <typename T, size_t I>
class ArrayStack {
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const pointer;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    auto constexpr begin() noexcept -> iterator { return &m_data[0]; }
    auto constexpr begin() const noexcept -> const_iterator { return &m_data[0]; }
    auto constexpr end() noexcept -> iterator { return begin() + m_size; }
    auto constexpr end() const noexcept -> const_iterator { return begin() + m_size; }
    auto constexpr cbegin() const noexcept -> const_iterator { return &m_data[0]; }
    auto constexpr cend() const noexcept -> const_iterator { return cbegin() + m_size; }
    auto constexpr front() -> reference { return m_data[0]; }
    auto constexpr front() const -> const_reference { return m_data[0]; }
    auto constexpr back() -> reference { return m_data[m_size - 1]; }
    auto constexpr back() const -> const_reference { return m_data[m_size - 1]; }
    auto constexpr size() noexcept -> size_type { return m_size; }
    auto constexpr max_size() noexcept -> size_type { return I; }
    auto constexpr push_back(value_type i) -> void { m_data[m_size++] = i; }
    auto constexpr pop_back() -> void { m_data[--m_size].~T(); }
    auto constexpr empty() -> bool { return !m_size; }

private:
    std::array<value_type, I> m_data = {};
    size_type m_size = 0;
};
