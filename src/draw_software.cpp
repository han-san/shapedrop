#include "draw_software.hpp"

#include "font.hpp"
#include "geometry.hpp"
#include "platform.hpp"
#include "positive.hpp"
#include "ui.hpp"

#include <concepts>
#include <thread>

namespace {

[[nodiscard]] auto alpha_blend_channel(const Positive<u8> bg, const Positive<u8> fg,
                                       const Positive<u8> alpha) -> Positive<u8> {
  const auto alphaRatio = u8 {alpha} / 255.;
  return Positive<u8> {(u8 {fg} * alphaRatio) + (u8 {bg} * (1 - alphaRatio))};
}

auto draw_pixel(void* data, const Color::RGBA color) -> void {
  auto* byte {static_cast<u8*>(data)};
  *byte = u8 {alpha_blend_channel(*byte, color.b, color.a)};
  ++byte;
  *byte = u8 {alpha_blend_channel(*byte, color.g, color.a)};
  ++byte;
  *byte = u8 {alpha_blend_channel(*byte, color.r, color.a)};
}

template <std::unsigned_integral ToType>
auto to_positive_in_range(const std::integral auto n, const std::size_t min, const std::size_t max)
    -> std::optional<Positive<ToType>> {
  return Positive<ToType>::try_construct(n).and_then(
      [min, max](auto posVal) -> std::optional<Positive<ToType>> {
        if (min <= posVal and max >= posVal) {
          return posVal;
        }
        return std::nullopt;
      });
}

auto draw_font_character(BackBuffer& buf, const FontCharacter& fontCharacter,
                         const Point<int> characterCoords) -> void {
  for (int y {0}; y < fontCharacter.dimensions.h; ++y) {
    const auto currYUnchecked = characterCoords.y + y + fontCharacter.yoff +
                                static_cast<int>(fontCharacter.ascent * fontCharacter.scale);
    auto maybeCurrY = to_positive_in_range<uint>(currYUnchecked, 0, uint {buf.dimensions.h});
    if (not maybeCurrY.has_value()) {
      continue;
    }
    const auto& currY = *maybeCurrY;

    for (int x {0}; x < fontCharacter.dimensions.w; ++x) {
      const auto currXUnchecked = characterCoords.x + x + fontCharacter.xoff;
      auto maybeCurrX = to_positive_in_range<uint>(currXUnchecked, 0, uint {buf.dimensions.w});
      if (not maybeCurrX.has_value()) {
        continue;
      }

      const auto& currX = *maybeCurrX;
      const auto currbyteindex = currY * buf.dimensions.w + currX;
      auto* currbyte = std::next(static_cast<u8*>(buf.memory), uint {currbyteindex * u8 {buf.bpp}});

      const auto relativeIndexUnchecked = (y * fontCharacter.dimensions.w) + x;
      const auto maybeRelativeIndex = Positive<uint>::try_construct(relativeIndexUnchecked);
      if (not maybeRelativeIndex.has_value()) {
        std::unreachable();
      }
      const auto relativeIndex = *maybeRelativeIndex;
      const auto a = fontCharacter.bitmap[uint {relativeIndex}];

      draw_pixel(currbyte, Color::RGBA {.r = 0U, .g = 0U, .b = 0U, .a = a});
    }
  }
}

auto draw_background_rows(BackBuffer bb, const Positive<std::size_t> startRow,
                          const Positive<std::size_t> endRow) {
  for (std::size_t y {startRow}; y < endRow; ++y) {
    for (std::size_t x {0}; x < uint {bb.dimensions.w}; ++x) {
      const Color::RGBA color {
          .r = static_cast<u8>(
              Color::RGBA::maxChannelValue *
              (static_cast<double>(x) / static_cast<double>(uint {bb.dimensions.w}))),
          .g = static_cast<u8>(
              Color::RGBA::maxChannelValue *
              (1 - (static_cast<double>(x) / static_cast<double>(uint {bb.dimensions.w})) *
                       (static_cast<double>(y) / static_cast<double>(uint {bb.dimensions.h})))),
          .b = static_cast<u8>(
              Color::RGBA::maxChannelValue *
              (static_cast<double>(y) / static_cast<double>(uint {bb.dimensions.h}))),
      };
      SoftwareRender::draw_solid_square(
          bb, {.x = static_cast<int>(x), .y = static_cast<int>(y), .w = 1, .h = 1}, color);
    }
  }
}

auto draw_playarea_rows(BackBuffer bb, const Positive<std::size_t> startRow,
                        const Positive<std::size_t> endRow, const Board& board,
                        const Positive<uint> positiveScale) {
  const uint scale {positiveScale};
  for (std::size_t y {startRow}; y < endRow; ++y) {
    for (std::size_t x {0}; x < Board::columns; ++x) {
      auto currindex = gsl::narrow_cast<gsl::index>((y * Board::columns) + x);
      const auto& block = board.block_at(currindex);
      auto color = block.isActive ? block.color : Color::black;
      Rect<int> square {.x = static_cast<int>((x + gPlayAreaDim.x) * scale),
                        .y = static_cast<int>((y - 2 + gPlayAreaDim.y) * scale),
                        .w = static_cast<int>(scale),
                        .h = static_cast<int>(scale)};
      SoftwareRender::draw_solid_square(bb, square, color);
    }
  }
}

} // namespace

namespace SoftwareRender {

auto draw_font_string(BackBuffer& buf, const FontString& fontString, Point<int> coords) -> void {
  for (const auto& fontCharacter : fontString.data) {
    draw_font_character(buf, fontCharacter, coords);
    coords.x += static_cast<int>(fontCharacter.advance);
  }
}

auto draw_font_string_normalized(BackBuffer& buf, const FontString& fontString,
                                 const Point<double> relativeCoords) -> void {
  const Point<int> realCoords {
      .x = static_cast<int>(relativeCoords.x * uint {buf.dimensions.w}),
      .y = static_cast<int>(relativeCoords.y * uint {buf.dimensions.h}),
  };
  draw_font_string(buf, fontString, realCoords);
}

auto draw_text(BackBuffer& buf, const std::string_view text, const Point<int> coords,
               const double pixelHeight) -> void {
  const auto fontString = FontString::from_height(text, pixelHeight);
  draw_font_string(buf, fontString, coords);
}

auto draw_text_normalized(BackBuffer& buf, const std::string_view text,
                          const Point<double> relativeCoords, const double pixelHeight) -> void {
  const Point<int> realCoords {
      .x = static_cast<int>(relativeCoords.x * uint {buf.dimensions.w}),
      .y = static_cast<int>(relativeCoords.y * uint {buf.dimensions.h}),
  };
  draw_text(buf, text, realCoords, pixelHeight * uint {buf.dimensions.h});
}

auto draw_solid_square(BackBuffer& buf, const Rect<int> sqr, const Color::RGBA color) -> void {
  for (int y {0}; y < sqr.h; ++y) {
    const auto pixely = static_cast<std::size_t>(sqr.y + y);
    if ((sqr.y + y) < 0 or pixely >= uint {buf.dimensions.h}) {
      continue;
    }
    for (int x {0}; x < sqr.w; ++x) {
      const auto pixelx = static_cast<std::size_t>(sqr.x + x);
      if ((sqr.x + x) < 0 or pixelx >= uint {buf.dimensions.w}) {
        continue;
      }

      const auto currbyteindex = (pixely * uint {buf.dimensions.w}) + pixelx;
      auto* currbyte {static_cast<u8*>(buf.memory) + (currbyteindex * u8 {buf.bpp})};

      draw_pixel(currbyte, color);
    }
  }
}

auto draw_solid_square_normalized(BackBuffer& buf, Rect<double> sqr, const Color::RGBA color)
    -> void {
  Rect<int> newSqr {.x = static_cast<int>(sqr.x * uint {buf.dimensions.w}),
                    .y = static_cast<int>(sqr.y * uint {buf.dimensions.h}),
                    .w = static_cast<int>(sqr.w * uint {buf.dimensions.w}),
                    .h = static_cast<int>(sqr.h * uint {buf.dimensions.h})};

  draw_solid_square(buf, newSqr, color);
}

auto draw_hollow_square(BackBuffer& buf, const Rect<int> sqr, const Color::RGBA color,
                        const int borderSize) -> void {
  for (int y {0}; y < sqr.h; ++y) {
    const auto pixely = static_cast<std::size_t>(sqr.y + y);
    if (sqr.y + y < 0 or pixely >= uint {buf.dimensions.h}) {
      continue;
    }
    for (int x {0}; x < sqr.w; ++x) {
      const auto pixelx = static_cast<std::size_t>(sqr.x + x);
      if (sqr.x + x < 0 or pixelx >= uint {buf.dimensions.w}) {
        continue;
      }

      // check if pixel is part of border
      if (not point_is_in_rect(Point<int> {.x = x, .y = y},
                               {.x = 0, .y = 0, .w = sqr.w, .h = sqr.h}) or
          point_is_in_rect(Point<int> {.x = x, .y = y}, {.x = borderSize,
                                                         .y = borderSize,
                                                         .w = sqr.w - (borderSize * 2),
                                                         .h = sqr.h - (borderSize * 2)})) {
        continue;
      }

      const auto currbyteindex = (pixely * uint {buf.dimensions.w}) + pixelx;
      auto* currbyte {static_cast<u8*>(buf.memory) + (currbyteindex * u8 {buf.bpp})};

      draw_pixel(currbyte, color);
    }
  }
}

auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr, const Color::RGBA color,
                                   const int borderSize) -> void {
  Rect<int> newSqr {.x = static_cast<int>(sqr.x * uint {buf.dimensions.w}),
                    .y = static_cast<int>(sqr.y * uint {buf.dimensions.h}),
                    .w = static_cast<int>(sqr.w * uint {buf.dimensions.w}),
                    .h = static_cast<int>(sqr.h * uint {buf.dimensions.h})};

  draw_hollow_square(buf, newSqr, color, borderSize);
}

/* auto draw_image(BackBuffer& backBuf, Point<int> const dest, BackBuffer& img)
 * -> void */
/* { */
/*     for (auto y {0}; y < img.h; ++y) { */
/*         auto const pixely {dest.y + y}; */
/*         if (pixely < 0 || pixely >= backBuf.h) { */
/*             continue; */
/*         } */
/*         for (auto x {0}; x < img.w; ++x) { */
/*             auto const pixelx {dest.x + x}; */
/*             if (pixelx < 0 || pixelx >= backBuf.w) { */
/*                 continue; */
/*             } */

/*             auto const currBBbyteindex {pixely * backBuf.w + pixelx}; */
/*             auto* currBBbyte {static_cast<u8*>(backBuf.memory) +
 * (currBBbyteindex * backBuf.bpp)}; */
/*             auto const currimgbyteindex {y * img.w + x}; */
/*             auto* currimgbyte {static_cast<u8*>(img.memory) +
 * (currimgbyteindex * img.bpp)}; */

/*             auto const r {*currimgbyte++}; */
/*             auto const g {*currimgbyte++}; */
/*             auto const b {*currimgbyte++}; */
/*             auto const a {*currimgbyte++}; */

/*             draw_pixel(currBBbyte, {r, g, b, a}); */
/*         } */
/*     } */
/* } */

auto draw(ProgramState& programState, GameState& gameState) -> void {
  auto bb = get_back_buffer();
  const auto scale = get_window_scale();
  const auto threadCount = std::thread::hardware_concurrency();
  std::vector<std::thread> threads {};
  threads.reserve(threadCount);

  const auto join_all = [](auto& threadContainer) {
    for (auto& thread : threadContainer) {
      thread.join();
    }
    threadContainer.clear();
  };

  // draw window background
  {
    const auto rowsPerThread = uint {bb.dimensions.h} / threadCount;
    for (std::size_t i {0}; i < threadCount - 1; ++i) {
      threads.emplace_back(&draw_background_rows, bb, rowsPerThread * i, rowsPerThread * (i + 1));
    }
    threads.emplace_back(&draw_background_rows, bb, rowsPerThread * (threadCount - 1),
                         bb.dimensions.h);
    join_all(threads);
  }

  switch (programState.levelType) {
  case ProgramState::LevelType::Menu: {
  } break;
  case ProgramState::LevelType::Game: {
    // draw playarea
    {
      const Positive<uint> positiveScale {scale};
      const auto rowsPerThread = (Board::rows - 2) / threadCount;
      for (std::size_t i {0}; i < threadCount - 1; ++i) {
        threads.emplace_back(&draw_playarea_rows, bb, 2 + (rowsPerThread * i),
                             2 + (rowsPerThread * (i + 1)), gameState.board, positiveScale);
      }
      threads.emplace_back(&draw_playarea_rows, bb, 2 + (rowsPerThread * (threadCount - 1)),
                           Board::rows, gameState.board, positiveScale);
      join_all(threads);
    }

    // draw currentShape and its shadow
    auto draw_shape_in_play_area = [&](Shape& shape) {
      for (auto& position : shape.get_absolute_block_positions()) {
        // since the top 2 rows shouldn't be visible, the y
        // position for drawing is 2 less than the shape's
        const auto actualYPosition = position.y - 2;

        // don't draw if square is above the playarea
        if (actualYPosition + gPlayAreaDim.y < gPlayAreaDim.y) {
          continue;
        }
        Rect<int> square {.x = (position.x + gPlayAreaDim.x) * scale,
                          .y = (actualYPosition + gPlayAreaDim.y) * scale,
                          .w = scale,
                          .h = scale};

        draw_solid_square(bb, square, shape.color);
      }
    };

    draw_shape_in_play_area(gameState.currentShapeShadow);
    draw_shape_in_play_area(gameState.currentShape);

    // draw shape previews
    const auto previewArray = gameState.shapePool.get_preview_shapes_array();
    auto i = 0;
    for (const auto shapeType : previewArray) {
      Shape shape {shapeType};
      shape.pos.x = gSidebarDim.x;
      const auto ySpacing = 3; // max height of a shape is 2 + 1 for a block of space
      shape.pos.y = gSidebarDim.y + ySpacing * i;
      for (auto& position : shape.get_absolute_block_positions()) {
        Rect<int> square {
            .x = (position.x) * scale, .y = (position.y) * scale, .w = scale, .h = scale};
        draw_solid_square(bb, square, shape.color);
      }
      ++i;
    }

    // draw held shape
    const auto holdShapeDim = gHoldShapeDim * scale;
    draw_solid_square(bb, holdShapeDim, Color::black);
    if (gameState.holdShapeType) {
      Shape shape {*gameState.holdShapeType};
      shape.pos = {};

      auto is_even = [](const auto n) { return (n % 2) == 0; };
      // offset to center shape inside hold square
      const auto shapeDimensions = shape.dimensions();
      const auto xOffset = is_even(gHoldShapeDim.w - shapeDimensions.w) ? 1.0 : 0.5;
      const auto yOffset = is_even(gHoldShapeDim.h - shapeDimensions.h) ? 0.0 : 0.5;

      for (auto& position : shape.get_absolute_block_positions()) {
        Rect<int> square {.x = static_cast<int>((position.x + gHoldShapeDim.x + xOffset) * scale),
                          .y = static_cast<int>((position.y + gHoldShapeDim.y + yOffset) * scale),
                          .w = scale,
                          .h = scale};
        draw_solid_square(bb, square, shape.color);
      }
    }
  } break;
  }

  UI::draw(bb);
}

} // namespace SoftwareRender
