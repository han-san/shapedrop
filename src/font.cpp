#include "font.hpp"

#include "platform.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

uchar static ttf_buffer[1<<25];
stbtt_fontinfo font;

auto init_font(std::string_view filePath) -> bool
{
    auto file = fopen(filePath.data(), "rb");
    if (!file) return false;
    fread(ttf_buffer, 1, 1<<25, file);
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(&(*ttf_buffer), 0));
    return true;
}

auto get_codepoint_kern_advance(char codepoint, char nextCodepoint, float scale) -> float
{
    int advance;
    int lsb;
    stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &lsb);
    return scale * (advance + stbtt_GetCodepointKernAdvance(&font, codepoint, nextCodepoint));
}

FontCharacter::FontCharacter(char c, float pixelHeight, char nextChar)
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

FontString::FontString(std::string string, float pixelHeight)
    // TODO: maybe move string? If so the parameter can't be used in the body
    : string{string}
{
    auto w = 0.f;

    auto size = string.size();
    data.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        auto c = string[i];
        auto nextChar = i + 1 == size ? 0 : string[i + 1];
        auto& fontCharacter = data.emplace_back(c, pixelHeight, nextChar);
        w += fontCharacter.advance;
    }

    auto windim = get_window_dimensions();
    normalizedH = pixelHeight / windim.h;
    normalizedW = w / windim.w;
}

auto FontString::from_width(std::string string, float desiredPixelWidth) -> FontString
{
    // start with a reasonable pixelheight value
    auto pixelHeight = 12.f;

    while (true) {
        auto width = 0.f;
        auto scale = stbtt_ScaleForPixelHeight(&font, pixelHeight);
        auto size = string.size();
        for (size_t i = 0; i < size; ++i) {
            auto c = string[i];
            auto nextChar = i + 1 == size ? 0 : string[i + 1];
            auto advance = get_codepoint_kern_advance(c, nextChar, scale);
            width += advance;
        }

        if (width > desiredPixelWidth) {
            pixelHeight -= 1.f;
        } else if (width < desiredPixelWidth - 2.f) {
            pixelHeight += 1.f;
        } else {
            break;
        }
    }

    // TODO: maybe move string here?
    return FontString(string, pixelHeight);
}

auto FontString::from_width_normalized(std::string string, float desiredWidth) -> FontString
{
    return from_width(string, get_window_dimensions().w * desiredWidth);
}