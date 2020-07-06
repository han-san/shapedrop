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

    constexpr V2Generic() = default;
    constexpr V2Generic(T const a, T const b): x{a}, y{b} {}
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

    constexpr V3Generic() = default;
    constexpr V3Generic(T const a, T const b, T const c): x{a}, y{b}, z{c} {}
};

template <typename T>
struct V4Generic {
    union {
        T x = 0;
        T r;
    };
    union {
        T y = 0;
        T g;
    };
    union {
        T w = 0;
        T b;
    };
    union {
        T h = 0;
        T a;
    };

    constexpr V4Generic() = default;
    constexpr V4Generic(T const a, T const b, T const c, T const d): x{a}, y{b}, w{c}, h{d} {}
};
using V2 = V2Generic<int>;
using V3 = V3Generic<int>;
using V4 = V4Generic<int>;
using V2f = V2Generic<float>;
using V3f = V3Generic<float>;
using V4f = V4Generic<float>;

using RGB = V3;
using RGBA = V4;

using Square = V4;
using Squaref = V4f;
using Position = V2;
using Positionf = V2f;

template <typename T, size_t I>
class ArrayStack {
    using ArrayType = std::array<T, I>;
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

    auto constexpr begin() noexcept -> iterator { return m_data.begin(); }
    auto constexpr begin() const noexcept -> const_iterator { return m_data.begin(); }
    auto constexpr end() noexcept -> iterator { return begin() + m_size; }
    auto constexpr end() const noexcept -> const_iterator { return begin() + m_size; }
    auto constexpr cbegin() const noexcept -> const_iterator { return m_data.cbegin(); }
    auto constexpr cend() const noexcept -> const_iterator { return cbegin() + m_size; }
    auto constexpr front() -> reference { return m_data.front(); }
    auto constexpr front() const -> const_reference { return m_data.front(); }
    auto constexpr back() -> reference { return m_data[m_size - 1]; }
    auto constexpr back() const -> const_reference { return m_data[m_size - 1]; }
    auto constexpr size() const noexcept -> size_type { return m_size; }
    auto constexpr max_size() const noexcept -> size_type { return I; }
    auto constexpr push_back(const_reference i) -> void { m_data[m_size++] = i; }
    auto constexpr push_back(value_type&& i) -> void { m_data[m_size++] = i; }
    auto constexpr pop_back() -> void { m_data[--m_size].~T(); }
    auto constexpr empty() const noexcept -> bool { return !m_size; }

private:
    ArrayType m_data;
    size_type m_size = 0;
};

auto inline point_is_in_rect(Positionf const point, Squaref const rect) {
    return (point.x >= rect.x) &&
        (point.x <= rect.x + rect.w) &&
        (point.y >= rect.y) &&
        (point.y <= rect.y + rect.h);
}

auto inline point_is_in_rect(Position const point, Square const rect) {
    return (point.x >= rect.x) &&
        (point.x <= rect.x + rect.w) &&
        (point.y >= rect.y) &&
        (point.y <= rect.y + rect.h);
}
