#include <string>
#include <string_view>

#include "font.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "platform.hpp"

uchar static ttf_buffer[1<<25];
stbtt_fontinfo font;
BakedChars static bakedChars;
BakedCharsBitmap bakedCharsBitmap {};

// FIXME: Temporary implementation. Should probably use std::path or something.
//        Also the path shouldn't be relative to the current working directory.
[[nodiscard]] auto static get_font_path() -> std::string
{
    return "./";
}

auto init_font(std::string const& fontName) -> bool
{
    auto const filePath = get_font_path() + fontName;
    auto* file {fopen(filePath.data(), "rb")};
    if (!file) { return false; }
    fread(ttf_buffer, 1, 1<<25, file);
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(&(*ttf_buffer), 0));

    auto constexpr fontHeight = 32;
    auto constexpr offset = 0;
    auto constexpr firstChar = 32;
    auto constexpr charCount = 96;
    stbtt_BakeFontBitmap(ttf_buffer, offset, fontHeight, bakedCharsBitmap.bitmap.data(), bakedCharsBitmap.w, bakedCharsBitmap.h, firstChar, charCount, bakedChars.data());

    return true;
}

[[nodiscard]] auto static get_codepoint_kern_advance(char const codepoint, char const nextCodepoint, double const scale) -> double
{
    int advance;
    int lsb;
    stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &lsb);
    return scale * static_cast<double>(advance + stbtt_GetCodepointKernAdvance(&font, codepoint, nextCodepoint));
}

FontCharacter::FontCharacter(char const c, double const pixelHeight, char const nextChar)
  : character(c),
    scale(static_cast<double>(stbtt_ScaleForPixelHeight(&font, static_cast<float>(pixelHeight)))),
    advance(get_codepoint_kern_advance(c, nextChar, scale))
{
    bitmap = stbtt_GetCodepointBitmap(&font, 0, static_cast<float>(scale), c, &dimensions.w, &dimensions.h, &xoff, &yoff);
    stbtt_GetFontVMetrics(&font, &ascent, nullptr, nullptr);
}

FontCharacter::~FontCharacter()
{
    stbtt_FreeBitmap(bitmap, font.userdata); // TODO: find out this actually does
}

auto get_baked_chars() -> BakedChars const& {
    return bakedChars;
}

auto get_baked_chars_bitmap() -> BakedCharsBitmap const& {
    return bakedCharsBitmap;
}

FontString::FontString(std::string_view const string, double const pixelHeight)
{
    auto w = 0.;

    auto const size = string.size();
    data.reserve(size);
    for (std::size_t i {0}; i < size; ++i) {
        auto const c = string[i];
        auto const nextChar = i + 1 == size ? '\0' : string[i + 1];
        auto const& fontCharacter = data.emplace_back(c, pixelHeight, nextChar);
        w += fontCharacter.advance;
    }

    auto const windim = get_window_dimensions();
    normalizedDimensions = {
        w / windim.w,
        pixelHeight / windim.h,
    };
}

auto FontString::get_text_width(std::string_view const text, double const fontHeight) -> double {
    auto width = 0.;
    auto const scale = static_cast<double>(stbtt_ScaleForPixelHeight(&font, static_cast<float>(fontHeight)));
    auto const size = text.size();
    for (std::size_t i {0}; i < size; ++i) {
        auto const c = text[i];
        auto const nextChar = (i + 1 == size) ? '\0' : text[i + 1];
        auto const advance = get_codepoint_kern_advance(c, nextChar, scale);
        width += advance;
    }

    return width;
}

auto FontString::get_text_width_normalized(std::string_view const text, double const fontHeightNormalized) -> double {
    return get_text_width(text, get_window_dimensions().h * fontHeightNormalized);
}

auto FontString::from_width(std::string_view const string, double const desiredPixelWidth) -> FontString
{
    // start with a reasonable pixelheight value
    auto pixelHeight = 12.;

    while (true) {
        auto width = get_text_width(string, pixelHeight);

        if (width > desiredPixelWidth) {
            pixelHeight -= 1.;
        } else if (width < desiredPixelWidth - 2.) {
            pixelHeight += 1.;
        } else {
            break;
        }
    }

    return FontString(string, pixelHeight);
}

auto FontString::from_width_normalized(std::string_view const string, double const desiredWidth) -> FontString
{
    return from_width(string, get_window_dimensions().w * desiredWidth);
}

auto FontString::from_height(std::string_view const string, double const desiredPixelHeight) -> FontString {
    return FontString(string, desiredPixelHeight);
}
auto FontString::from_height_normalized(std::string_view const string, double const desiredHeight) -> FontString {
    return from_height(string, get_window_dimensions().h * desiredHeight);
}
