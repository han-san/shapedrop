#pragma once

#include <string_view>
#include <vector>

#include <SDL.h>
#include <SDL_opengl.h>

#include "../jint.h"

#include "../core.hpp"
#include "../input.hpp"
#include "../util.hpp"

namespace platform::SDL {

enum class RenderMode {
    software,
    opengl
};

auto swap_buffer() -> void;
auto get_back_buffer() -> BackBuffer;
auto get_window_scale() -> int;
auto change_window_scale(int) -> void;
auto get_window_dimensions() -> Rect<int>::Size;
auto get_event() -> Event;
auto get_gl_context() -> SDL_GLContext;
auto get_render_mode() -> RenderMode;

}
