#pragma once

#include <string_view>
#include <vector>

#include "../jint.h"

#include "../core.hpp"
#include "../input.hpp"
#include "../util.hpp"

namespace platform::SDL {

auto swap_buffer() -> void;
auto get_back_buffer() -> BackBuffer;
auto get_window_scale() -> int;
auto change_window_scale(int) -> void;
auto get_window_dimensions() -> Rect<int>::Size;
auto get_event() -> Event;

}
