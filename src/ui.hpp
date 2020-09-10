#pragma once

#include <string>
#include <string_view>

#include "core.hpp"
#include "util.hpp"

namespace UI {
    // Sometimes the coordinates and sizes are relative to the entire window
    // and sometimes they are relative to the current menu's region and its
    // children's regions. These two classes make sure that values of one scale
    // aren't accidentally assigned to variables of the other scale.
    class WindowScale {
        double value;
    public:
        WindowScale() noexcept = default;
        constexpr explicit operator double() const { return value; };
        constexpr WindowScale(double const from) noexcept : value{from} {};

        auto constexpr operator +=(WindowScale const& rhs) -> WindowScale& {
            value += double(rhs);
            return *this;
        }
        auto constexpr friend operator +(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs += rhs;
        }
        auto constexpr operator -=(WindowScale const& rhs) -> WindowScale& {
            value -= double(rhs);
            return *this;
        }
        auto constexpr friend operator -(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs -= rhs;
        }
        auto constexpr operator *=(WindowScale const& rhs) -> WindowScale& {
            value *= double(rhs);
            return *this;
        }
        auto constexpr friend operator *(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs *= rhs;
        }
        auto constexpr operator /=(WindowScale const& rhs) -> WindowScale& {
            value /= double(rhs);
            return *this;
        }
        auto constexpr friend operator /(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs /= rhs;
        }
    };

    class RelativeScale {
        double value;
    public:
        RelativeScale() noexcept = default;
        constexpr explicit operator double() const { return value; };
        constexpr RelativeScale(double const from) noexcept : value{from} {};

        auto constexpr operator +=(RelativeScale const& rhs) -> RelativeScale& {
            value += double(rhs);
            return *this;
        }
        auto constexpr friend operator +(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs += rhs;
        }
        auto constexpr operator -=(RelativeScale const& rhs) -> RelativeScale& {
            value -= double(rhs);
            return *this;
        }
        auto constexpr friend operator -(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs -= rhs;
        }
        auto constexpr operator *=(RelativeScale const& rhs) -> RelativeScale& {
            value *= double(rhs);
            return *this;
        }
        auto constexpr friend operator *(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs *= rhs;
        }
        auto constexpr operator /=(RelativeScale const& rhs) -> RelativeScale& {
            value /= double(rhs);
            return *this;
        }
        auto constexpr friend operator /(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs /= rhs;
        }
    };

    using WindowScalePoint = V2Generic<WindowScale>;
    using WindowScaleRect = V4Generic<WindowScale>;
    using RelativeScalePoint = V2Generic<RelativeScale>;
    using RelativeScaleRect = V4Generic<RelativeScale>;

    enum class XAlignment {
        LEFT,
        CENTER,
        RIGHT
    };

    auto label(std::string text, WindowScale fontHeight, RelativeScalePoint offset) -> void;
    auto label(std::string text, WindowScale fontHeight, XAlignment xAlign, RelativeScale yOffset = 0.f) -> void;
    auto button(std::string text, WindowScale fontHeight, RelativeScalePoint pos) -> bool;
    auto button(std::string text, WindowScale fontHeight, XAlignment xAlign, RelativeScale yOffset = 0.f) -> bool;
    auto spinbox(std::string_view text, WindowScale fontHeight, RelativeScalePoint offset, size_t& value, size_t minValue, size_t maxValue) -> void;
    auto spinbox(std::string_view text, WindowScale fontHeight, XAlignment xAlign, RelativeScale yOffset, size_t& value, size_t minValue, size_t maxValue) -> void;
    auto begin_menu(RelativeScaleRect region, RGBA color = {}) -> void;
    auto begin_menu(RelativeScaleRect region, RGB color, u8 alpha = Color::Alpha::opaque) -> void;
    auto end_menu() -> void;
    auto draw(BackBuffer bb) -> void;
    auto update_state(Message message) -> void;
}
