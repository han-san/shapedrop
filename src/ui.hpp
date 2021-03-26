#pragma once

#include "core.hpp"
#include "input.hpp"
#include "util.hpp"

#include <string>
#include <string_view>

namespace UI {
    // Sometimes the coordinates and sizes are relative to the entire window
    // and sometimes they are relative to the current menu's region and its
    // children's regions. These two classes make sure that values of one scale
    // aren't accidentally assigned to variables of the other scale.
    class WindowScale {
    public:
        WindowScale() noexcept = default;
        [[nodiscard]] constexpr explicit operator double() const { return value; }
        constexpr WindowScale(double const from) noexcept : value {from} {}

        auto constexpr operator +=(WindowScale const& rhs) -> WindowScale& {
            value += double {rhs};
            return *this;
        }
        [[nodiscard]] auto constexpr friend operator +(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs += rhs;
        }
        auto constexpr operator -=(WindowScale const& rhs) -> WindowScale& {
            value -= double {rhs};
            return *this;
        }
        [[nodiscard]] auto constexpr friend operator -(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs -= rhs;
        }
        auto constexpr operator *=(WindowScale const& rhs) -> WindowScale& {
            value *= double {rhs};
            return *this;
        }
        [[nodiscard]] auto constexpr friend operator *(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs *= rhs;
        }
        auto constexpr operator /=(WindowScale const& rhs) -> WindowScale& {
            value /= double {rhs};
            return *this;
        }
        [[nodiscard]] auto constexpr friend operator /(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs /= rhs;
        }
    private:
        double value;
    };

    class RelativeScale {
    public:
        RelativeScale() noexcept = default;
        [[nodiscard]] constexpr explicit operator double() const { return value; }
        constexpr RelativeScale(double const from) noexcept : value {from} {}

        auto constexpr operator +=(RelativeScale const& rhs) -> RelativeScale& {
            value += double {rhs};
            return *this;
        }
        [[nodiscard]] auto constexpr friend operator +(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs += rhs;
        }
        auto constexpr operator -=(RelativeScale const& rhs) -> RelativeScale& {
            value -= double {rhs};
            return *this;
        }
        [[nodiscard]] auto constexpr friend operator -(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs -= rhs;
        }
        auto constexpr operator *=(RelativeScale const& rhs) -> RelativeScale& {
            value *= double {rhs};
            return *this;
        }
        [[nodiscard]] auto constexpr friend operator *(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs *= rhs;
        }
        auto constexpr operator /=(RelativeScale const& rhs) -> RelativeScale& {
            value /= double {rhs};
            return *this;
        }
        [[nodiscard]] auto constexpr friend operator /(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs /= rhs;
        }
    private:
        double value;
    };

    using WindowScalePoint = Point<WindowScale>;
    using WindowScaleRect = Rect<WindowScale>;
    using RelativeScalePoint = Point<RelativeScale>;
    using RelativeScaleRect = Rect<RelativeScale>;

    enum class XAlignment {
        Left,
        Center,
        Right
    };

    auto label(std::string text, WindowScale fontHeight, RelativeScalePoint offset) -> void;
    auto label(std::string text, WindowScale fontHeight, XAlignment xAlign, RelativeScale yOffset = 0.) -> void;
    [[nodiscard]] auto button(std::string text, WindowScale fontHeight, RelativeScalePoint offset) -> bool;
    [[nodiscard]] auto button(std::string text, WindowScale fontHeight, XAlignment xAlign, RelativeScale yOffset = 0.) -> bool;
    auto spinbox(std::string_view text, WindowScale fontHeight, RelativeScalePoint offset, int& value, int minValue, int maxValue) -> void;
    auto spinbox(std::string_view text, WindowScale fontHeight, XAlignment xAlign, RelativeScale yOffset, int& value, int minValue, int maxValue) -> void;
    auto begin_menu(RelativeScaleRect region, Color::RGBA color = Color::transparent) -> void;
    auto end_menu() -> void;
    auto draw(BackBuffer bb) -> void;
    auto update_state(Event event) -> void;
}
