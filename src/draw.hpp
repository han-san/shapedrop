#pragma once

#include "core.hpp"
#include "font.hpp"
#include "platform.hpp"
#include "util.hpp"

#include <string_view>

[[nodiscard]] inline auto to_screen_space(const Rect<double> square) -> Rect<double> {
  const auto dim = get_window_dimensions();
  return {square.x * dim.w, square.y * dim.h, square.w * dim.w, square.h * dim.h};
}

[[nodiscard]] inline auto to_normalized(const Rect<double> square) -> Rect<double> {
  const auto dim = get_window_dimensions();
  return {square.x / dim.w, square.y / dim.h, square.w / dim.w, square.h / dim.h};
}

[[nodiscard]] inline auto to_normalized_height(const double height) -> double {
  return height / get_window_dimensions().h;
}

[[nodiscard]] inline auto to_normalized_width(const double width) -> double {
  return width / get_window_dimensions().w;
}

[[nodiscard]] inline auto to_screen_space_height(const double height) -> double {
  return height * get_window_dimensions().h;
}

[[nodiscard]] inline auto to_screen_space_width(const double width) -> double {
  return width * get_window_dimensions().w;
}

auto draw(ProgramState& programState, GameState& gameState) -> void;

auto draw_solid_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color) -> void;
auto draw_solid_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color) -> void;
auto draw_hollow_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color, int borderSize = 1)
    -> void;
auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color,
                                   int borderSize = 1) -> void;
auto draw_font_string(BackBuffer& buf, const FontString& fontString, Point<int> coords) -> void;
auto draw_font_string_normalized(BackBuffer& buf, const FontString& fontString,
                                 Point<double> relativeCoords) -> void;
auto draw_text(BackBuffer& buf, std::string_view text, Point<int> coords, double pixelHeight)
    -> void;
auto draw_text_normalized(BackBuffer& buf, std::string_view text, Point<double> relativeCoords,
                          double pixelHeight) -> void;
