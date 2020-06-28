#pragma once

#include <string_view>
#include <vector>

#include "../jint.h"

#include "../core.hpp"

namespace platform::SDL {

auto swap_buffer() -> void;
auto get_back_buffer() -> BackBuffer;
auto get_window_scale() -> int;
auto change_window_scale(int) -> void;
auto get_window_dimensions() -> V2;
auto handle_input() -> Message;

}
