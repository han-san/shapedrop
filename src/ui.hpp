#pragma once

#include <string>

#include "core.hpp"
#include "util.hpp"

namespace UI {
    auto label(std::string text, float fontSize, Positionf pos, bool centered = false) -> void;
    auto button(std::string text, float fontSize, Positionf pos, bool centered = false) -> bool;
    auto begin_menu(std::string text, float fontSize, Squaref region) -> void;
    auto end_menu() -> void;
    auto draw(BackBuffer bb) -> void;
    auto update_state(Message message) -> void;
}
