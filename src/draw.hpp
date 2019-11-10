#pragma once

#include "platform.hpp"
#include "font.hpp"

auto inline to_screen_space(Squaref square) -> Squaref {
    auto bb = get_back_buffer();
    return { square.x * bb.w, square.y * bb.h, square.w * bb.w, square.h * bb.h };
}

auto inline to_normalized(Squaref square) -> Squaref {
    auto bb = get_back_buffer();
    return { square.x / bb.w, square.y / bb.h, square.w / bb.w, square.h / bb.h };
}

auto draw_solid_square(BackBuffer& buf, Squaref sqr, RGB color, uint a = 0xff) -> void;
auto draw_solid_square_normalized(BackBuffer& buf, Squaref sqr, RGB color, uint a = 0xff) -> void;
auto draw_hollow_square(BackBuffer& buf, Squaref sqr, RGB color, int a = 0xff, int borderSize = 1) -> void;
auto draw_hollow_square_normalized(BackBuffer& buf, Squaref sqr, RGB color, int a = 0xff, int borderSize = 1) -> void;
auto draw_font_string(BackBuffer& buf, FontString& fontString, int x, int y) -> void;
auto draw_font_string_normalized(BackBuffer& buf, FontString& fontString, float x, float y) -> void;
auto draw_text(BackBuffer& buf, std::string_view text, int x, int y, float pixelHeight) -> void;
auto draw_text_normalized(BackBuffer& buf, std::string_view text, float x, float y, float pixelHeight) -> void;
