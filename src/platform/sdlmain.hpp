#pragma once

#include <string_view>
#include <vector>

#include "../jint.h"

#include "../core.hpp"
#include "../input.hpp"
#include "../util.hpp"
#include "../draw_opengl.hpp"

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
auto get_render_mode() -> RenderMode;
auto get_opengl_render_context() -> OpenGLRender::Context const&;

}
