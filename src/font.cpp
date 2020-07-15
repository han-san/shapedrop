#include <string>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "platform.hpp"

#include "font.hpp"

uchar static ttf_buffer[1<<25];
stbtt_fontinfo font;

// FIXME: Temporary implementation. Should probably use std::path or something.
//        Also the path shouldn't be relative to the current working directory.
auto get_font_path() -> std::string
{
    return "./";
}

auto init_font(std::string const fontName) -> bool
{
    auto const filePath = get_font_path() + std::move(fontName);
    auto file = fopen(filePath.data(), "rb");
    if (!file) return false;
    fread(ttf_buffer, 1, 1<<25, file);
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(&(*ttf_buffer), 0));
    return true;
}

auto get_codepoint_kern_advance(char const codepoint, char const nextCodepoint, float const scale) -> float
{
    int advance;
    int lsb;
    stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &lsb);
    return scale * (advance + stbtt_GetCodepointKernAdvance(&font, codepoint, nextCodepoint));
}

FontCharacter::FontCharacter(char const c, float const pixelHeight, char const nextChar)
  : character(c),
    scale(stbtt_ScaleForPixelHeight(&font, pixelHeight)),
    advance(get_codepoint_kern_advance(c, nextChar, scale))
{
    bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, &xoff, &yoff);
    stbtt_GetFontVMetrics(&font, &ascent, 0, 0);
}

FontCharacter::~FontCharacter()
{
    stbtt_FreeBitmap(bitmap, font.userdata); // TODO: find out this actually does
}

FontString::FontString(std::string_view const string, float const pixelHeight)
{
    auto w = 0.f;

    auto const size = string.size();
    data.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        auto const c = string[i];
        auto const nextChar = i + 1 == size ? '\0' : string[i + 1];
        auto const& fontCharacter = data.emplace_back(c, pixelHeight, nextChar);
        w += fontCharacter.advance;
    }

    auto const windim = get_window_dimensions();
    normalizedH = pixelHeight / windim.h;
    normalizedW = w / windim.w;
}

auto FontString::get_text_width(std::string_view const text, float const fontHeight) -> float {
    auto width = 0.f;
    auto const scale = stbtt_ScaleForPixelHeight(&font, fontHeight);
    auto const size = text.size();
    for (size_t i = 0; i < size; ++i) {
        auto const c = text[i];
        auto const nextChar = (i + 1 == size) ? '\0' : text[i + 1];
        auto const advance = get_codepoint_kern_advance(c, nextChar, scale);
        width += advance;
    }

    return width;
}

auto FontString::get_text_width_normalized(std::string_view const text, float const fontHeightNormalized) -> float {
    return get_text_width(text, get_window_dimensions().h * fontHeightNormalized);
}

auto FontString::from_width(std::string_view const string, float const desiredPixelWidth) -> FontString
{
    // start with a reasonable pixelheight value
    auto pixelHeight = 12.f;

    while (true) {
        auto width = get_text_width(string, pixelHeight);

        if (width > desiredPixelWidth) {
            pixelHeight -= 1.f;
        } else if (width < desiredPixelWidth - 2.f) {
            pixelHeight += 1.f;
        } else {
            break;
        }
    }

    return FontString(string, pixelHeight);
}

auto FontString::from_width_normalized(std::string_view const string, float const desiredWidth) -> FontString
{
    return from_width(string, get_window_dimensions().w * desiredWidth);
}
