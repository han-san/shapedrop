#pragma once

#include <string>
#include <vector>
#include <string_view>

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
    float normalizedW;
    float normalizedH;

    auto static from_width(std::string_view string, float desiredPixelWidth) -> FontString;
    auto static from_width_normalized(std::string_view string, float desiredWidth) -> FontString;
    auto static from_height(std::string_view const string, float const desiredPixelHeight) -> FontString {
        return FontString(string, desiredPixelHeight);
    }
    auto static from_height_normalized(std::string_view const string, float const desiredHeight) -> FontString {
        return from_height(string, get_window_dimensions().h * desiredHeight);
    }
    auto static get_text_width(std::string_view text, float fontSize) -> float;
    auto static get_text_width_normalized(std::string_view text, float fontSize) -> float;

private:
    FontString(std::string_view string, float pixelHeight);
};

auto get_codepoint_kern_advance(char codepoint, char nextCodepoint, float scale) -> float;
auto init_font(std::string const& filePath) -> bool;
