#pragma once

#include <string_view>

#include "platform.hpp"
#include "font.hpp"

auto inline to_screen_space(Squaref const square) -> Squaref {
    auto const dim = get_window_dimensions();
    return { square.x * dim.w, square.y * dim.h, square.w * dim.w, square.h * dim.h };
}

auto inline to_normalized(Squaref const square) -> Squaref {
    auto const dim = get_window_dimensions();
    return { square.x / dim.w, square.y / dim.h, square.w / dim.w, square.h / dim.h };
}

auto inline to_normalized_height(float const height) -> float {
    return height / get_window_dimensions().h;
}

auto inline to_normalized_width(float const width) -> float {
    return width / get_window_dimensions().w;
}

auto inline to_screen_space_height(float const height) -> float {
    return height * get_window_dimensions().h;
}

auto inline to_screen_space_width(float const width) -> float {
    return width * get_window_dimensions().w;
}

auto draw_solid_square(BackBuffer& buf, Squaref const sqr, RGB const color, int const a = 0xff) -> void;
auto draw_solid_square_normalized(BackBuffer& buf, Squaref sqr, RGB const color, int const a = 0xff) -> void;
auto draw_hollow_square(BackBuffer& buf, Squaref const sqr, RGB const color, int const a = 0xff, int const borderSize = 1) -> void;
auto draw_hollow_square_normalized(BackBuffer& buf, Squaref sqr, RGB const color, int const a = 0xff, int const borderSize = 1) -> void;
auto draw_font_string(BackBuffer& buf, FontString const& fontString, int x, int const y) -> void;
auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString, float const x, float const y) -> void;
auto draw_text(BackBuffer& buf, std::string_view const text, int const x, int const y, float const pixelHeight) -> void;
auto draw_text_normalized(BackBuffer& buf, std::string_view const text, float const x, float const y, float const pixelHeight) -> void;
