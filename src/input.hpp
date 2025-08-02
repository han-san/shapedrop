#pragma once

#include "core.hpp"

#include <cstdint>

struct Event {
  enum class Type : std::uint8_t {
    None,
    Quit,
    Reset,
    Hold,
    Move_right,
    Move_left,
    Increase_speed,
    Reset_speed,
    Drop,
    Rotate_left,
    Rotate_right,
    Increase_window_size,
    Decrease_window_size,
    Mousebuttondown,
    Pause,
  };

  Type type;
  Point<int> mouseCoords;
};

auto handle_input(ProgramState& programState, GameState& gameState) -> void;
