#pragma once

#include <vector>
#include <string_view>

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
    float advance;

    FontCharacter(char c, float pixelHeight, char nextChar);
    ~FontCharacter();

    FontCharacter(FontCharacter&&) = default;
    FontCharacter& operator=(FontCharacter&&) = default;

    FontCharacter& operator=(FontCharacter const&) = delete;
    FontCharacter(FontCharacter const&) = delete;
};

auto create_font_string(std::string_view string, float pixelHeight) -> std::vector<FontCharacter>;
auto get_codepoint_kern_advance(char codepoint, char nextCodepoint, float scale) -> float;
auto swap_buffer() -> void;
auto get_back_buffer() -> BackBuffer;
auto get_window_scale() -> int;
auto change_window_scale(int) -> void;
auto get_window_dimensions() -> V2;
auto handle_input() -> Message;

}
