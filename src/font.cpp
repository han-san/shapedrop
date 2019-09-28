#include "font.hpp"

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
  : scale(stbtt_ScaleForPixelHeight(&font, pixelHeight)),
    advance(get_codepoint_kern_advance(c, nextChar, scale))
{
    bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, &xoff, &yoff);
    stbtt_GetFontVMetrics(&font, &ascent, 0, 0);
}

FontCharacter::~FontCharacter()
{
    stbtt_FreeBitmap(bitmap, font.userdata); // TODO: find out this actually does
}

FontString::FontString(std::string_view string, float pixelHeight)
    : h{pixelHeight}
{
    auto size = string.size();
    data.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        auto c = string[i];
        auto nextChar = i + 1 == size ? 0 : string[i + 1];
        auto& fontCharacter = data.emplace_back(c, pixelHeight, nextChar);
        w += fontCharacter.advance;
    }
}

