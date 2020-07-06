#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "jint.h"

#include "platform.hpp"


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

    FontCharacter(char const c, float const pixelHeight, char const nextChar);
    ~FontCharacter();

    FontCharacter(FontCharacter&&) = default;
    FontCharacter& operator=(FontCharacter&&) = default;

    FontCharacter& operator=(FontCharacter const&) = delete;
    FontCharacter(FontCharacter const&) = delete;
};

class FontString {
public:
    std::vector<FontCharacter> data;
    float normalizedW;
    float normalizedH;

    auto static from_width(std::string_view const string, float const desiredPixelWidth) -> FontString;
    auto static from_width_normalized(std::string_view const string, float const desiredWidth) -> FontString;
    auto static from_height(std::string_view const string, float const desiredPixelHeight) -> FontString {
        return FontString(string, desiredPixelHeight);
    }
    auto static from_height_normalized(std::string_view const string, float const desiredHeight) -> FontString {
        return from_height(string, get_window_dimensions().h * desiredHeight);
    }
    auto static get_text_width(std::string_view const text, float const fontSize) -> float;
    auto static get_text_width_normalized(std::string_view const text, float const fontSize) -> float;

private:
    FontString(std::string_view const string, float const pixelHeight);
};

auto get_codepoint_kern_advance(char const codepoint, char const nextCodepoint, float const scale) -> float;
auto init_font(std::string_view const filePath) -> bool;
