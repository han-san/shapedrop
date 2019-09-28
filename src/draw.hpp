#pragma once

#include "platform.hpp"
#include "font.hpp"

struct Square {
    float x;
    float y;
    float w;
    float h;

    auto to_screen_space() -> Square {
        auto bb = get_back_buffer();
        return { x * bb.w, y * bb.h, w * bb.w, h * bb.h };
    }

    auto to_normalized() -> Square {
        auto bb = get_back_buffer();
        return { x / bb.w, y / bb.h, w / bb.w, h / bb.h };
    }
};

using Point = V2f;

auto draw_solid_square(BackBuffer& buf, Square sqr, uint r, uint g, uint b, uint a = 0xff) -> void;
auto draw_text_normalized(BackBuffer& buf, std::string_view text, float x, float y, float pixelHeight) -> void;
auto draw_hollow_square(BackBuffer& buf, Square sqr, int r, int g, int b, int a = 0xff, int borderSize = 1) -> void;
auto draw_font_string(BackBuffer& buf, FontString& fontString, int x, int y) -> void;
auto draw_text(BackBuffer& buf, std::string_view text, int x, int y, float pixelHeight) -> void;
