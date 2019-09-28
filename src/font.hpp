#pragma once

#include <string_view>
#include <vector>

#include "jint.h"

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
auto init_font(std::string_view filePath) -> bool;
