#pragma once

#include "core.hpp"
#include "font.hpp"

namespace SoftwareRender {

auto draw(ProgramState& programState, GameState& gameState) -> void;

auto draw_solid_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color) -> void;
auto draw_solid_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color) -> void;
auto draw_hollow_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color, int borderSize = 1) -> void;
auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color, int borderSize = 1) -> void;
auto draw_font_string(BackBuffer& buf, FontString const& fontString, Point<int> coords) -> void;
auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString, Point<double> relativeCoords) -> void;
auto draw_text(BackBuffer& buf, std::string_view text, Point<int> coords, double pixelHeight) -> void;
auto draw_text_normalized(BackBuffer& buf, std::string_view text, Point<double> relativeCoords, double pixelHeight) -> void;

}
