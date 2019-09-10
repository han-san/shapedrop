#pragma once

#include <array>

template <typename T>
struct V2Generic {
    union {
        T w = 0;
        T x;
    };
    union {
        T h = 0;
        T y;
    };

    V2Generic() = default;
    V2Generic(T a, T b): x{a}, y{b} {}
};

template <typename T>
struct V3Generic {
    union {
        T x = 0;
        T r;
    };
    union {
        T y = 0;
        T g;
    };
    union {
        T z = 0;
        T b;
    };

    V3Generic() = default;
    V3Generic(T a, T b, T c): x{a}, y{b}, z{c} {}
};

using V2 = V2Generic<int>;
using V3 = V3Generic<int>;
using V2f = V2Generic<float>;
using V3f = V3Generic<float>;

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
