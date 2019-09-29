#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "platform.hpp"

#include "jint.h"

class FontCharacter {
public:
    uchar* bitmap;
    int w, h;
    int xoff, yoff;
    int ascent;
    char character;
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
    std::string string;
    std::vector<FontCharacter> data;
    float normalizedW = 0;
    float normalizedH;

    auto static from_width(std::string string, float desiredPixelWidth) -> FontString;
    auto static from_width_normalized(std::string string, float desiredWidth) -> FontString;
    auto static from_height(std::string string, float desiredPixelHeight) -> FontString {
        return FontString(string, desiredPixelHeight);
    }
    auto static from_height_normalized(std::string string, float desiredHeight) -> FontString {
        return from_height(string, get_window_dimensions().h * desiredHeight);
    }

private:
    FontString(std::string string, float pixelHeight);
};

auto get_codepoint_kern_advance(char codepoint, char nextCodepoint, float scale) -> float;
auto init_font(std::string_view filePath) -> bool;
