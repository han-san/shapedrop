#pragma once

#include <string>

#include "core.hpp"
#include "util.hpp"

namespace UI {
    // Sometimes the coordinates and sizes are relative to the entire window
    // and sometimes they are relative to the current menu's region and its
    // children's regions. These two classes make sure that values of one scale
    // aren't accidentally assigned to variables of the other scale.
    class WindowScale {
        float value;
    public:
        WindowScale() = default;
        constexpr explicit operator float() const { return value; };
        constexpr WindowScale(float const from) : value{from} {};

        auto constexpr operator +=(WindowScale const& rhs) -> WindowScale& {
            value += float(rhs);
            return *this;
        }
        auto constexpr friend operator +(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs += rhs;
        }
        auto constexpr operator -=(WindowScale const& rhs) -> WindowScale& {
            value -= float(rhs);
            return *this;
        }
        auto constexpr friend operator -(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs -= rhs;
        }
        auto constexpr operator *=(WindowScale const& rhs) -> WindowScale& {
            value *= float(rhs);
            return *this;
        }
        auto constexpr friend operator *(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs *= rhs;
        }
        auto constexpr operator /=(WindowScale const& rhs) -> WindowScale& {
            value /= float(rhs);
            return *this;
        }
        auto constexpr friend operator /(WindowScale lhs, WindowScale const& rhs) -> WindowScale {
            return lhs /= rhs;
        }
    };

    class RelativeScale {
        float value;
    public:
        RelativeScale() = default;
        constexpr explicit operator float() const { return value; };
        constexpr RelativeScale(float const from) : value{from} {};

        auto constexpr operator +=(RelativeScale const& rhs) -> RelativeScale& {
            value += float(rhs);
            return *this;
        }
        auto constexpr friend operator +(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs += rhs;
        }
        auto constexpr operator -=(RelativeScale const& rhs) -> RelativeScale& {
            value -= float(rhs);
            return *this;
        }
        auto constexpr friend operator -(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs -= rhs;
        }
        auto constexpr operator *=(RelativeScale const& rhs) -> RelativeScale& {
            value *= float(rhs);
            return *this;
        }
        auto constexpr friend operator *(RelativeScale lhs, RelativeScale const& rhs) -> RelativeScale {
            return lhs *= rhs;
        }
        auto constexpr operator /=(RelativeScale const& rhs) -> RelativeScale& {
            value /= float(rhs);
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

    auto label(std::string const text, WindowScale const fontHeight, RelativeScalePoint const offset, bool centered = false) -> void;
    auto button(std::string const text, WindowScale const fontHeight, RelativeScalePoint const pos, bool const centered = false) -> bool;
    auto begin_menu(RelativeScaleRect const region) -> void;
    auto end_menu() -> void;
    auto draw(BackBuffer bb) -> void;
    auto update_state(Message const message) -> void;
}
