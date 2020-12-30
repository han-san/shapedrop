#pragma once

#include <string>
#include <vector>
#include <string_view>

#include "jint.h"

#include "platform.hpp"

class FontCharacter {
public:
    uchar* bitmap;
    Rect<int>::Size dimensions;
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
    Rect<double>::Size normalizedDimensions;

    [[nodiscard]] auto static from_width(std::string_view string, double desiredPixelWidth) -> FontString;
    [[nodiscard]] auto static from_width_normalized(std::string_view string, double desiredWidth) -> FontString;
    [[nodiscard]] auto static from_height(std::string_view const string, double const desiredPixelHeight) -> FontString {
        return FontString(string, desiredPixelHeight);
    }
    [[nodiscard]] auto static from_height_normalized(std::string_view const string, double const desiredHeight) -> FontString {
        return from_height(string, get_window_dimensions().h * desiredHeight);
    }
    [[nodiscard]] auto static get_text_width(std::string_view text, double fontHeight) -> double;
    [[nodiscard]] auto static get_text_width_normalized(std::string_view text, double fontHeightNormalized) -> double;

private:
    FontString(std::string_view string, double pixelHeight);
};

auto init_font(std::string const& fontName) -> bool;
