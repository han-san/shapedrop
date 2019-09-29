#include <cassert>

#include "core.hpp"

#include "draw.hpp"

auto alpha_blend_channel(int bg, int fg, int alpha) -> float
{
    assert(bg >= 0 && bg <= 255);
    assert(fg >= 0 && fg <= 255);
    assert(alpha >= 0 && alpha <= 255);

    auto alphaRatio = alpha / 255.f;
    return fg * alphaRatio + bg * (1 - alphaRatio);
}

auto draw_font_character(BackBuffer& buf, FontCharacter& fontCharacter, int realX, int realY) -> void
{
    for (auto y = 0; y < fontCharacter.h; ++y) {
        auto currY = realY + y + fontCharacter.yoff + (int)(fontCharacter.ascent * fontCharacter.scale);
        if (currY < 0 || currY >= buf.h) continue;
        for (auto x = 0; x < fontCharacter.w; ++x) {
            auto currX = realX + x + fontCharacter.xoff;
            if (currX < 0 || currX >= buf.w) continue;

            auto currbyteindex = currY * buf.w + currX;
            auto currbyte = ((u8*)buf.memory + currbyteindex * buf.bpp);

            auto relativeIndex = y * fontCharacter.w + x;
            auto a = fontCharacter.bitmap[relativeIndex];

            *currbyte = alpha_blend_channel(*currbyte, 0, a);
            ++currbyte;
            *currbyte = alpha_blend_channel(*currbyte, 0, a);
            ++currbyte;
            *currbyte = alpha_blend_channel(*currbyte, 0, a);
        }
    }
}

auto draw_font_string(BackBuffer& buf, FontString& fontString, int x, int y) -> void
{
    for (auto& fontCharacter : fontString.data) {
        draw_font_character(buf, fontCharacter, x, y);
        x += fontCharacter.advance;
    }
}

auto draw_font_string_normalized(BackBuffer& buf, FontString& fontString, float x, float y) -> void
{
    x *= buf.w;
    y *= buf.h;

    draw_font_string(buf, fontString, x, y);
}

auto draw_text(BackBuffer& buf, std::string_view text, int x, int y, float pixelHeight) -> void
{
    auto fontString = FontString::from_height(std::string(text), pixelHeight);
    draw_font_string(buf, fontString, x, y);
}

auto draw_text_normalized(BackBuffer& buf, std::string_view text, float x, float y, float pixelHeight) -> void
{
    x *= buf.w;
    y *= buf.h;
    pixelHeight *= buf.h;

    draw_text(buf, text, x, y, pixelHeight);
}

auto draw_solid_square(BackBuffer& buf, Square sqr, RGB color, uint a) -> void
{
    for (auto y = 0; y < sqr.h; ++y) {
        auto pixely = (int)sqr.y + y;
        if (pixely < 0 || pixely >= buf.h) {
            continue;
        }
        for (auto x = 0; x < sqr.w; ++x) {
            auto pixelx = (int)sqr.x + x;
            if (pixelx < 0 || pixelx >= buf.w) {
                continue;
            }

            auto currbyteindex = pixely * buf.w + pixelx;
            auto currbyte = ((u8*)buf.memory + currbyteindex * buf.bpp);

            *currbyte = alpha_blend_channel(*currbyte, color.b, a);
            ++currbyte;
            *currbyte = alpha_blend_channel(*currbyte, color.g, a);
            ++currbyte;
            *currbyte = alpha_blend_channel(*currbyte, color.r, a);
        }
    }
}

auto draw_solid_square_normalized(BackBuffer& buf, Square sqr, RGB color, uint a) -> void
{
    sqr.x *= buf.w;
    sqr.y *= buf.h;
    sqr.w *= buf.w;
    sqr.h *= buf.h;

    draw_solid_square(buf, sqr, color, a);
}

auto draw_hollow_square(BackBuffer& buf, Square sqr, RGB color, int a, int borderSize) -> void
{
    for (auto y = 0; y < sqr.h; ++y) {
        auto pixely = (int)sqr.y + y;
        if (pixely < 0 || pixely >= buf.h) {
            continue;
        }
        for (auto x = 0; x < sqr.w; ++x) {
            auto pixelx = (int)sqr.x + x;
            if (pixelx < 0 || pixelx >= buf.w) {
                continue;
            }

            // check if pixel is part of border
            if (((x < 0 || x >= borderSize) && (x >= sqr.w || x < sqr.w - borderSize)) &&
                ((y < 0 || y >= borderSize) && (y >= sqr.h || y < sqr.h - borderSize)))
            {
                continue;
            }

            auto currbyteindex = pixely * buf.w + pixelx;
            auto currbyte = ((u8*)buf.memory + currbyteindex * buf.bpp);

            auto alpha_blend = [](uint bg, uint fg, uint alpha) {
                auto alphaRatio = alpha / 255.0;
                return fg * alphaRatio + bg * (1 - alphaRatio);
            };

            *currbyte = alpha_blend(*currbyte, color.b, a);
            ++currbyte;
            *currbyte = alpha_blend(*currbyte, color.g, a);
            ++currbyte;
            *currbyte = alpha_blend(*currbyte, color.r, a);
        }
    }
}

auto draw_hollow_square_normalized(BackBuffer& buf, Square sqr, RGB color, int a, int borderSize) -> void
{
    sqr.x *= buf.w;
    sqr.y *= buf.h;
    sqr.w *= buf.w;
    sqr.h *= buf.h;

    draw_hollow_square(buf, sqr, color, a, borderSize);
}

auto draw_image(BackBuffer& backBuf, Point dest, BackBuffer& img) -> void
{
    for (auto y = 0; y < img.h; ++y) {
        auto pixely = (int)dest.y + y;
        if (pixely < 0 || pixely >= backBuf.h) {
            continue;
        }
        for (auto x = 0; x < img.w; ++x) {
            auto pixelx = (int)dest.x + x;
            if (pixelx < 0 || pixelx >= backBuf.w) {
                continue;
            }

            auto currBBbyteindex = pixely * backBuf.w + pixelx;
            auto currBBbyte = ((u8*)backBuf.memory + currBBbyteindex * backBuf.bpp);
            auto currimgbyteindex = y * img.w + x;
            auto currimgbyte = ((u8*)img.memory + currimgbyteindex * img.bpp);

            auto r = *currimgbyte++;
            auto g = *currimgbyte++;
            auto b = *currimgbyte++;
            auto a = *currimgbyte++;

            // FIXME: hack
            if (!a) {
                continue;
            }

            *currBBbyte++ = b;
            *currBBbyte++ = g;
            *currBBbyte++ = r;
        }
    }
}
