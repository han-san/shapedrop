#include <cassert>
#include <string_view>

#include "core.hpp"

#include "draw.hpp"

auto alpha_blend_channel(int const bg, int const fg, int const alpha) -> double
{
    assert(bg >= 0 && bg <= 255);
    assert(fg >= 0 && fg <= 255);
    assert(alpha >= 0 && alpha <= 255);

    auto const alphaRatio = alpha / 255.f;
    return fg * alphaRatio + bg * (1 - alphaRatio);
}

auto draw_pixel(void* data, RGBA const color) -> void
{
    auto byte = (u8*) data;
    *byte = u8(alpha_blend_channel(*byte, color.b, color.a));
    ++byte;
    *byte = u8(alpha_blend_channel(*byte, color.g, color.a));
    ++byte;
    *byte = u8(alpha_blend_channel(*byte, color.r, color.a));
}

auto draw_font_character(BackBuffer& buf, FontCharacter const& fontCharacter, int const realX, int const realY) -> void
{
    for (auto y = 0; y < fontCharacter.h; ++y) {
        auto const currY = realY + y + fontCharacter.yoff + (int)(fontCharacter.ascent * fontCharacter.scale);
        if (currY < 0 || currY >= buf.h) continue;
        for (auto x = 0; x < fontCharacter.w; ++x) {
            auto const currX = realX + x + fontCharacter.xoff;
            if (currX < 0 || currX >= buf.w) continue;

            auto const currbyteindex = currY * buf.w + currX;
            auto currbyte = ((u8*)buf.memory + currbyteindex * buf.bpp);

            auto const relativeIndex = y * fontCharacter.w + x;
            auto const a = fontCharacter.bitmap[relativeIndex];

            draw_pixel(currbyte, {0, 0, 0, a});
        }
    }
}

auto draw_font_string(BackBuffer& buf, FontString const& fontString, int x, int const y) -> void
{
    for (auto const& fontCharacter : fontString.data) {
        draw_font_character(buf, fontCharacter, x, y);
        x += int(fontCharacter.advance);
    }
}

auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString, double const x, double const y) -> void
{
    draw_font_string(buf, fontString, int(x * buf.w), int(y * buf.h));
}

auto draw_text(BackBuffer& buf, std::string_view const text, int const x, int const y, double const pixelHeight) -> void
{
    auto const fontString = FontString::from_height(text, pixelHeight);
    draw_font_string(buf, fontString, x, y);
}

auto draw_text_normalized(BackBuffer& buf, std::string_view const text, double const x, double const y, double const pixelHeight) -> void
{
    draw_text(buf, text, int(x * buf.w), int(y * buf.h), pixelHeight * buf.h);
}

auto draw_solid_square(BackBuffer& buf, Squaref const sqr, RGB const color, int const a) -> void
{
    for (auto y = 0; y < sqr.h; ++y) {
        auto const pixely = (int)sqr.y + y;
        if (pixely < 0 || pixely >= buf.h) {
            continue;
        }
        for (auto x = 0; x < sqr.w; ++x) {
            auto const pixelx = (int)sqr.x + x;
            if (pixelx < 0 || pixelx >= buf.w) {
                continue;
            }

            auto const currbyteindex = pixely * buf.w + pixelx;
            auto currbyte = ((u8*)buf.memory + currbyteindex * buf.bpp);

            draw_pixel(currbyte, {color.r, color.g, color.b, a});
        }
    }
}

auto draw_solid_square_normalized(BackBuffer& buf, Squaref sqr, RGB const color, int const a) -> void
{
    sqr.x *= buf.w;
    sqr.y *= buf.h;
    sqr.w *= buf.w;
    sqr.h *= buf.h;

    draw_solid_square(buf, sqr, color, a);
}

auto draw_hollow_square(BackBuffer& buf, Squaref const sqr, RGB const color, int const a, int const borderSize) -> void
{
    for (auto y = 0; y < sqr.h; ++y) {
        auto const pixely = (int)sqr.y + y;
        if (pixely < 0 || pixely >= buf.h) {
            continue;
        }
        for (auto x = 0; x < sqr.w; ++x) {
            auto const pixelx = (int)sqr.x + x;
            if (pixelx < 0 || pixelx >= buf.w) {
                continue;
            }

            // check if pixel is part of border
            if (((x < 0 || x >= borderSize) && (x >= sqr.w || x < sqr.w - borderSize)) &&
                ((y < 0 || y >= borderSize) && (y >= sqr.h || y < sqr.h - borderSize)))
            {
                continue;
            }

            auto const currbyteindex = pixely * buf.w + pixelx;
            auto currbyte = ((u8*)buf.memory + currbyteindex * buf.bpp);

            draw_pixel(currbyte, {color.r, color.g, color.b, a});
        }
    }
}

auto draw_hollow_square_normalized(BackBuffer& buf, Squaref sqr, RGB const color, int const a, int const borderSize) -> void
{
    sqr.x *= buf.w;
    sqr.y *= buf.h;
    sqr.w *= buf.w;
    sqr.h *= buf.h;

    draw_hollow_square(buf, sqr, color, a, borderSize);
}

auto draw_image(BackBuffer& backBuf, Position const dest, BackBuffer& img) -> void
{
    for (auto y = 0; y < img.h; ++y) {
        auto const pixely = (int)dest.y + y;
        if (pixely < 0 || pixely >= backBuf.h) {
            continue;
        }
        for (auto x = 0; x < img.w; ++x) {
            auto const pixelx = (int)dest.x + x;
            if (pixelx < 0 || pixelx >= backBuf.w) {
                continue;
            }

            auto const currBBbyteindex = pixely * backBuf.w + pixelx;
            auto currBBbyte = ((u8*)backBuf.memory + currBBbyteindex * backBuf.bpp);
            auto const currimgbyteindex = y * img.w + x;
            auto currimgbyte = ((u8*)img.memory + currimgbyteindex * img.bpp);

            auto const r = *currimgbyte++;
            auto const g = *currimgbyte++;
            auto const b = *currimgbyte++;
            auto const a = *currimgbyte++;

            // FIXME: hack
            if (!a) {
                continue;
            }

            draw_pixel(currBBbyte, {r, g, b, a});
        }
    }
}
