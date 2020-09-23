#pragma once

#include <string_view>

#include "platform.hpp"
#include "font.hpp"
#include "util.hpp"
#include "core.hpp"

auto inline to_screen_space(Rect<double> const square) -> Rect<double> {
    auto const dim {get_window_dimensions()};
    return { square.x * dim.w, square.y * dim.h, square.w * dim.w, square.h * dim.h };
}

auto inline to_normalized(Rect<double> const square) -> Rect<double> {
    auto const dim {get_window_dimensions()};
    return { square.x / dim.w, square.y / dim.h, square.w / dim.w, square.h / dim.h };
}

auto inline to_normalized_height(double const height) -> double {
    return height / get_window_dimensions().h;
}

auto inline to_normalized_width(double const width) -> double {
    return width / get_window_dimensions().w;
}

auto inline to_screen_space_height(double const height) -> double {
    return height * get_window_dimensions().h;
}

auto inline to_screen_space_width(double const width) -> double {
    return width * get_window_dimensions().w;
}

auto draw_solid_square(BackBuffer& buf, Rect<double> sqr, Color::RGBA color) -> void;
auto draw_solid_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color) -> void;
auto draw_hollow_square(BackBuffer& buf, Rect<double> sqr, Color::RGBA color, int borderSize = 1) -> void;
auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color, int borderSize = 1) -> void;
auto draw_font_string(BackBuffer& buf, FontString const& fontString, int x, int y) -> void;
auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString, double x, double y) -> void;
auto draw_text(BackBuffer& buf, std::string_view text, int x, int y, double pixelHeight) -> void;
auto draw_text_normalized(BackBuffer& buf, std::string_view text, double x, double y, double pixelHeight) -> void;
auto draw(ProgramState& programState, GameState& gameState) -> void;
