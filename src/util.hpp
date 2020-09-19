#pragma once

#include <array>

#include "jint.h"

template <typename T>
struct V2Generic {
    union {
        T w {0};
        T x;
    };
    union {
        T h {0};
        T y;
    };

    constexpr V2Generic() = default;
    constexpr V2Generic(T const a, T const b): x{a}, y{b} {}

    using ThisType = V2Generic<T>;

    auto constexpr operator +=(ThisType const& rhs) -> ThisType& {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    auto constexpr friend operator +(ThisType lhs, ThisType const& rhs) -> ThisType {
        return lhs += rhs;
    }
};

template <typename T>
struct V3Generic {
    union {
        T x {0};
        T r;
    };
    union {
        T y {0};
        T g;
    };
    union {
        T z {0};
        T b;
    };

    constexpr V3Generic() = default;
    constexpr V3Generic(T const a, T const b, T const c): x{a}, y{b}, z{c} {}

    using ThisType = V3Generic<T>;

    auto constexpr operator +=(ThisType const& rhs) -> ThisType& {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    auto constexpr friend operator +(ThisType lhs, ThisType const& rhs) -> ThisType {
        return lhs += rhs;
    }
};

template <typename T>
struct V4Generic {
    union {
        T x {0};
        T r;
    };
    union {
        T y {0};
        T g;
    };
    union {
        T w {0};
        T b;
    };
    union {
        T h {0};
        T a;
    };

    constexpr V4Generic() = default;
    constexpr V4Generic(T const a, T const b, T const c, T const d): x{a}, y{b}, w{c}, h{d} {}

    using ThisType = V4Generic<T>;

    auto constexpr operator +=(ThisType const& rhs) -> ThisType& {
        x += rhs.x;
        y += rhs.y;
        w += rhs.w;
        h += rhs.h;
        return *this;
    }
    auto constexpr friend operator +(ThisType lhs, ThisType const& rhs) -> ThisType {
        return lhs += rhs;
    }
};

using V2 = V2Generic<int>;
using V3 = V3Generic<int>;
using V4 = V4Generic<int>;
using V2f = V2Generic<double>;
using V3f = V3Generic<double>;
using V4f = V4Generic<double>;

namespace Color {
    struct RGBA {
        u8 static constexpr maxChannelValue{0xFF};
        struct Alpha {
            u8 static constexpr opaque {maxChannelValue};
            u8 static constexpr transparent {0};
        };

        constexpr RGBA() = default;
        constexpr RGBA(int const rr, int const gg, int const bb)
            : r{rr}, g{gg}, b{bb}
        {}
        constexpr RGBA(int const rr, int const gg, int const bb, int const aa)
            : r{rr}, g{gg}, b{bb}, a{aa}
        {}

        int r {0};
        int g {0};
        int b {0};
        int a {Alpha::opaque};
    };

    RGBA static constexpr red {0xFF, 0, 0};
    RGBA static constexpr green {0, 0xFF, 0};
    RGBA static constexpr blue {0, 0, 0xFF};
    RGBA static constexpr cyan {0, 0xFF, 0xFF};
    RGBA static constexpr transparent {0, 0, 0, RGBA::Alpha::transparent};

    struct Shape {
        RGBA static constexpr I {0, 0xF0, 0xF0};
        RGBA static constexpr O {0xF0, 0xF0, 0};
        RGBA static constexpr L {0xF0, 0xA0, 0};
        RGBA static constexpr J {0, 0, 0xF0};
        RGBA static constexpr S {0, 0xF0, 0};
        RGBA static constexpr Z {0xF0, 0, 0};
        RGBA static constexpr T {0xA0, 0, 0xF0};
    };
}

using Square = V4;
using Squaref = V4f;
using Position = V2;
using Positionf = V2f;

template <typename T, std::size_t I>
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
    size_type m_size {0};
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
