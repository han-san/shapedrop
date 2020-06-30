#pragma once

#include <string>

#include "core.hpp"
#include "util.hpp"

namespace UI {
    auto label(std::string const text, float const fontHeight, Positionf pos, bool centered = false) -> void;
    auto button(std::string const text, float const fontHeight, Positionf const pos, bool const centered = false) -> bool;
    auto begin_menu(Squaref const region) -> void;
    auto end_menu() -> void;
    auto draw(BackBuffer bb) -> void;
    auto update_state(Message const message) -> void;
}
