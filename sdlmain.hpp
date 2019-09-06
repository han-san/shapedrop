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
    int ascent;
    // advance depends on scale and must be declared after
    float scale;
    float advance;

    FontCharacter(char c, float pixelHeight, char nextChar);
    ~FontCharacter();

    FontCharacter(FontCharacter&&) = default;
    FontCharacter& operator=(FontCharacter&&) = default;

    FontCharacter& operator=(FontCharacter const&) = delete;
    FontCharacter(FontCharacter const&) = delete;
};

class FontString {
public:
    std::vector<FontCharacter> data;
    float w = 0;
    float h;

    FontString(std::string_view string, float pixelHeight);
};

auto get_codepoint_kern_advance(char codepoint, char nextCodepoint, float scale) -> float;
auto swap_buffer() -> void;
auto get_back_buffer() -> BackBuffer;
auto get_window_scale() -> int;
auto change_window_scale(int) -> void;
auto get_window_dimensions() -> V2;
auto handle_input() -> Message;

}
