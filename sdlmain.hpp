#pragma once

#include "core.hpp"
#include "jint.h"

namespace platform::SDL {

class FontCharacter {
public:
    uchar* bitmap;
    int w, h;
    int xoff, yoff;
    float scale;
    int ascent;

    FontCharacter(char c, float pixelHeight);
    ~FontCharacter();
};

auto get_codepoint_kern_advance(char codepoint, char nextCodepoint, float scale) -> float;
auto swap_buffer() -> void;
auto get_back_buffer() -> BackBuffer;
auto get_window_scale() -> int;
auto change_window_scale(int) -> void;
auto get_window_dimensions() -> V2;
auto handle_input() -> Message;

}
