#pragma once

#include <array>
#include <cassert>

#include "jint.h"

template <typename T>
class PositiveGeneric {
    using ThisType = PositiveGeneric<T>;
public:
    PositiveGeneric() = default;
    PositiveGeneric(schar i)
        : PositiveGeneric {sllong {i}}
    {}
    PositiveGeneric(sshort i)
        : PositiveGeneric {sllong {i}}
    {}
    PositiveGeneric(sint i)
        : PositiveGeneric {sllong {i}}
    {}
    PositiveGeneric(slong i)
        : PositiveGeneric {sllong {i}}
    {}
    PositiveGeneric(sllong i)
        : m_value {static_cast<T>(i)}
    {
        // make sure it fits the type's range
        assert(i >= 0);
        assert(static_cast<T>(-1) >= i);
    }
    PositiveGeneric(uchar i)
        : PositiveGeneric {ullong {i}}
    {}
    PositiveGeneric(ushort i)
        : PositiveGeneric {ullong {i}}
    {}
    PositiveGeneric(uint i)
        : PositiveGeneric {ullong {i}}
    {}
    PositiveGeneric(ulong i)
        : PositiveGeneric {ullong {i}}
    {}
    PositiveGeneric(ullong i)
        : m_value{static_cast<T>(i)}
    {
        assert(static_cast<T>(-1) >= i);
    }
    explicit PositiveGeneric(double i)
        : m_value {static_cast<T>(i)}
    {
        // make sure it fits the type's range
        assert(i >= 0);
        assert(static_cast<T>(-1) >= static_cast<ullong>(i));
    }

    explicit operator T() const noexcept { return m_value; }
    explicit operator bool() const noexcept { return m_value; }

    auto constexpr operator -=(ThisType const& rhs) -> ThisType& {
        auto const rhsVal = static_cast<T>(rhs);
        assert(m_value >= rhsVal);
        m_value -= rhsVal;
        return *this;
    }
    auto constexpr friend operator -(ThisType lhs, ThisType const& rhs) -> ThisType {
        return lhs -= rhs;
    }
    auto constexpr operator +=(ThisType const& rhs) -> ThisType& {
        auto const rhsVal = static_cast<T>(rhs);
        m_value += rhsVal;
        return *this;
    }
    auto constexpr friend operator +(ThisType lhs, ThisType const& rhs) -> ThisType {
        return lhs += rhs;
    }
    auto constexpr operator *=(ThisType const& rhs) -> ThisType& {
        auto const rhsVal = static_cast<T>(rhs);
        m_value *= rhsVal;
        return *this;
    }
    auto constexpr friend operator *(ThisType lhs, ThisType const& rhs) -> ThisType {
        return lhs *= rhs;
    }
    auto constexpr operator /=(ThisType const& rhs) -> ThisType& {
        auto const rhsVal = static_cast<T>(rhs);
        m_value /= rhsVal;
        return *this;
    }
    auto constexpr friend operator /(ThisType lhs, ThisType const& rhs) -> ThisType {
        return lhs /= rhs;
    }

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
    T x {};
    T y {};
    T z {};

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

using V2 = V2Generic<int>;
using V3 = V3Generic<int>;
using V2f = V2Generic<double>;
using V3f = V3Generic<double>;

namespace Color {
    struct RGBA {
        u8 static constexpr maxChannelValue{0xFF};
        struct Alpha {
            u8 static constexpr opaque {maxChannelValue};
            u8 static constexpr transparent {0};
        };

        // This struct is meant to be used only for constexpr RGBA variables,
        // since the compiler can then guarantee the members are initialized to
        // valid values.
        // The regular RGBA's members can't be constexpr since PositiveU8
        // asserts that its members are valid in its constructors.
        struct Unchecked {
            u8 r {0};
            u8 g {0};
            u8 b {0};
            u8 a {RGBA::Alpha::opaque};

            operator RGBA() const { return {r, g, b, a}; }
        };

        PositiveU8 r {0};
        PositiveU8 g {0};
        PositiveU8 b {0};
        PositiveU8 a {Alpha::opaque};
    };

    RGBA::Unchecked static constexpr red {0xFF, 0, 0};
    RGBA::Unchecked static constexpr green {0, 0xFF, 0};
    RGBA::Unchecked static constexpr blue {0, 0, 0xFF};
    RGBA::Unchecked static constexpr cyan {0, 0xFF, 0xFF};
    RGBA::Unchecked static constexpr white {0xFF, 0xFF, 0xFF};
    RGBA::Unchecked static constexpr black {0, 0, 0};
    RGBA::Unchecked static constexpr transparent {0, 0, 0, RGBA::Alpha::transparent};

    // An invalid color to give some visual feedback when a color hasn't been
    // properly initialized. White isn't really used otherwise in the game, so
    // hopefully it will be obvious that something is wrong.
    RGBA::Unchecked static constexpr invalid {white};

    struct Shape {
        RGBA::Unchecked static constexpr I {0, 0xF0, 0xF0};
        RGBA::Unchecked static constexpr O {0xF0, 0xF0, 0};
        RGBA::Unchecked static constexpr L {0xF0, 0xA0, 0};
        RGBA::Unchecked static constexpr J {0, 0, 0xF0};
        RGBA::Unchecked static constexpr S {0, 0xF0, 0};
        RGBA::Unchecked static constexpr Z {0xF0, 0, 0};
        RGBA::Unchecked static constexpr T {0xA0, 0, 0xF0};
    };
}

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
};

template <typename T>
struct Point {
    T x {};
    T y {};

    auto constexpr operator +=(V2Generic<T> const& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    auto constexpr friend operator +(Point<T> lhs, V2Generic<T> const& rhs) {
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

    [[nodiscard]] auto constexpr begin() noexcept -> iterator { return m_data.begin(); }
    [[nodiscard]] auto constexpr begin() const noexcept -> const_iterator { return m_data.begin(); }
    [[nodiscard]] auto constexpr end() noexcept -> iterator { return begin() + m_size; }
    [[nodiscard]] auto constexpr end() const noexcept -> const_iterator { return begin() + m_size; }
    [[nodiscard]] auto constexpr cbegin() const noexcept -> const_iterator { return m_data.cbegin(); }
    [[nodiscard]] auto constexpr cend() const noexcept -> const_iterator { return cbegin() + m_size; }
    [[nodiscard]] auto constexpr front() -> reference { return m_data.front(); }
    [[nodiscard]] auto constexpr front() const -> const_reference { return m_data.front(); }
    [[nodiscard]] auto constexpr back() -> reference { return m_data[m_size - 1]; }
    [[nodiscard]] auto constexpr back() const -> const_reference { return m_data[m_size - 1]; }
    [[nodiscard]] auto constexpr size() const noexcept -> size_type { return m_size; }
    [[nodiscard]] auto constexpr max_size() const noexcept -> size_type { return maxSize; }
    [[nodiscard]] auto constexpr empty() const noexcept -> bool { return !m_size; }
    auto constexpr push_back(const_reference i) -> void {
        assert(m_size < maxSize);
        m_data[m_size++] = i;
    }
    auto constexpr push_back(value_type&& i) -> void {
        assert(m_size < maxSize);
        m_data[m_size++] = std::move(i);
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
auto point_is_in_rect(Point<T> const& point, Rect<T> const& rect) {
    return (point.x >= rect.x) &&
        (point.x < rect.x + rect.w) &&
        (point.y >= rect.y) &&
        (point.y < rect.y + rect.h);
}

// This won't work if T doesn't have a default constructor. If it does have
// one, there's a potential performance hit from default constructing every
// element and then immediately reassigning all of them.
template <typename T, std::size_t N>
auto constexpr make_filled_array(T const& val) {
    std::array<T, N> arr;
    arr.fill(val);
    return arr;
}
