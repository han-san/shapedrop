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
    double scale;
    double advance;

    FontCharacter(char c, double pixelHeight, char nextChar);
    ~FontCharacter();

    FontCharacter(FontCharacter&&) = default;
    auto operator=(FontCharacter&&) -> FontCharacter& = default;

    auto operator=(FontCharacter const&) -> FontCharacter& = delete;
    FontCharacter(FontCharacter const&) = delete;
};

class FontString {
public:
    std::vector<FontCharacter> data;
    double normalizedW;
    double normalizedH;

    auto static from_width(std::string_view string, double desiredPixelWidth) -> FontString;
    auto static from_width_normalized(std::string_view string, double desiredWidth) -> FontString;
    auto static from_height(std::string_view const string, double const desiredPixelHeight) -> FontString {
        return FontString(string, desiredPixelHeight);
    }
    auto static from_height_normalized(std::string_view const string, double const desiredHeight) -> FontString {
        return from_height(string, get_window_dimensions().h * desiredHeight);
    }
    auto static get_text_width(std::string_view text, double fontSize) -> double;
    auto static get_text_width_normalized(std::string_view text, double fontSize) -> double;

private:
    FontString(std::string_view string, double pixelHeight);
};

auto init_font(std::string const& filePath) -> bool;
