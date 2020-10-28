#include <cassert>
#include <string_view>
#include <thread>

#include "core.hpp"
#include "ui.hpp"
#include "util.hpp"

#include "draw.hpp"

auto static alpha_blend_channel(PositiveU8 const bg, PositiveU8 const fg, PositiveU8 const alpha) -> PositiveU8
{
    auto const alphaRatio {u8 {alpha} / 255.};
    return static_cast<PositiveU8>(u8 {fg} * alphaRatio + u8 {bg} * (1 - alphaRatio));
}

auto static draw_pixel(void* data, Color::RGBA const color) -> void
{
    auto* byte {static_cast<u8*>(data)};
    *byte = u8 {alpha_blend_channel(PositiveU8 {*byte}, color.b, color.a)};
    ++byte;
    *byte = u8 {alpha_blend_channel(PositiveU8 {*byte}, color.g, color.a)};
    ++byte;
    *byte = u8 {alpha_blend_channel(PositiveU8 {*byte}, color.r, color.a)};
}

auto static draw_font_character(BackBuffer& buf, FontCharacter const& fontCharacter, int const realX, int const realY) -> void
{
    for (auto y {0}; y < fontCharacter.h; ++y) {
        auto const currY {realY + y + fontCharacter.yoff + static_cast<int>(fontCharacter.ascent * fontCharacter.scale)};
        if (currY < 0 || static_cast<uint>(currY) >= uint {buf.h}) { continue; }
        for (auto x {0}; x < fontCharacter.w; ++x) {
            auto const currX {realX + x + fontCharacter.xoff};
            if (currX < 0 || static_cast<uint>(currX) >= uint {buf.w}) { continue; }

            auto const currbyteindex {static_cast<uint>(currY) * uint {buf.w} + static_cast<uint>(currX)};
            auto* currbyte {static_cast<u8*>(buf.memory) + (currbyteindex * u8 {buf.bpp})};

            auto const relativeIndex {static_cast<std::size_t>(y * fontCharacter.w + x)};
            auto const a {fontCharacter.bitmap[relativeIndex]};

            draw_pixel(currbyte, Color::RGBA {0u, 0u, 0u, a});
        }
    }
}

auto draw_font_string(BackBuffer& buf, FontString const& fontString, int x, int const y) -> void
{
    for (auto const& fontCharacter : fontString.data) {
        draw_font_character(buf, fontCharacter, x, y);
        x += static_cast<int>(fontCharacter.advance);
    }
}

auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString, double const x, double const y) -> void
{
    auto const realX = static_cast<int>(x * uint {buf.w});
    auto const realY = static_cast<int>(y * uint {buf.h});
    draw_font_string(buf, fontString, realX, realY);
}

auto draw_text(BackBuffer& buf, std::string_view const text, int const x, int const y, double const pixelHeight) -> void
{
    auto const fontString {FontString::from_height(text, pixelHeight)};
    draw_font_string(buf, fontString, x, y);
}

auto draw_text_normalized(BackBuffer& buf, std::string_view const text, double const x, double const y, double const pixelHeight) -> void
{
    auto const realX = static_cast<int>(x * uint {buf.w});
    auto const realY = static_cast<int>(y * uint {buf.h});
    draw_text(buf, text, realX, realY, pixelHeight * uint {buf.h});
}

auto draw_solid_square(BackBuffer& buf, Rect<int> const sqr, Color::RGBA const color) -> void
{
    for (auto y {0}; y < sqr.h; ++y) {
        auto const pixely {static_cast<std::size_t>(sqr.y + y)};
        if ((sqr.y + y) < 0 || pixely >= buf.h) {
            continue;
        }
        for (auto x {0}; x < sqr.w; ++x) {
            auto const pixelx {static_cast<std::size_t>(sqr.x + x)};
            if ((sqr.x + x) < 0 || pixelx >= buf.w) {
                continue;
            }

            auto const currbyteindex {pixely * uint {buf.w} + pixelx};
            auto* currbyte {static_cast<u8*>(buf.memory) + (currbyteindex * u8 {buf.bpp})};

            draw_pixel(currbyte, color);
        }
    }
}

auto draw_solid_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA const color) -> void
{
    Rect<int> newSqr {
        static_cast<int>(sqr.x * uint {buf.w}),
        static_cast<int>(sqr.y * uint {buf.h}),
        static_cast<int>(sqr.w * uint {buf.w}),
        static_cast<int>(sqr.h * uint {buf.h})
    };

    draw_solid_square(buf, newSqr, color);
}

auto draw_hollow_square(BackBuffer& buf, Rect<int> const sqr, Color::RGBA const color, int const borderSize) -> void
{
    for (int y {0}; y < sqr.h; ++y) {
        auto const pixely {static_cast<std::size_t>(sqr.y + y)};
        if (sqr.y + y < 0 || pixely >= buf.h) {
            continue;
        }
        for (int x {0}; x < sqr.w; ++x) {
            auto const pixelx {static_cast<std::size_t>(sqr.x + x)};
            if (sqr.x + x < 0 || pixelx >= buf.w) {
                continue;
            }

            // check if pixel is part of border
            if (!point_is_in_rect(Point<int> {x, y}, {0, 0, sqr.w, sqr.h}) || point_is_in_rect(Point<int> {x, y}, {borderSize, borderSize, sqr.w - borderSize * 2, sqr.h - borderSize * 2}))
            {
                continue;
            }

            auto const currbyteindex {pixely * uint {buf.w} + pixelx};
            auto* currbyte {static_cast<u8*>(buf.memory) + (currbyteindex * u8 {buf.bpp})};

            draw_pixel(currbyte, color);
        }
    }
}

auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA const color, int const borderSize) -> void
{
    Rect<int> newSqr {
        static_cast<int>(sqr.x * uint {buf.w}),
        static_cast<int>(sqr.y * uint {buf.h}),
        static_cast<int>(sqr.w * uint {buf.w}),
        static_cast<int>(sqr.h * uint {buf.h})
    };

    draw_hollow_square(buf, newSqr, color, borderSize);
}

/* auto draw_image(BackBuffer& backBuf, Point<int> const dest, BackBuffer& img) -> void */
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
/*             auto* currBBbyte {static_cast<u8*>(backBuf.memory) + (currBBbyteindex * backBuf.bpp)}; */
/*             auto const currimgbyteindex {y * img.w + x}; */
/*             auto* currimgbyte {static_cast<u8*>(img.memory) + (currimgbyteindex * img.bpp)}; */

/*             auto const r {*currimgbyte++}; */
/*             auto const g {*currimgbyte++}; */
/*             auto const b {*currimgbyte++}; */
/*             auto const a {*currimgbyte++}; */

/*             draw_pixel(currBBbyte, {r, g, b, a}); */
/*         } */
/*     } */
/* } */

auto static draw_background_rows(BackBuffer bb, PositiveSize_t const startRow, PositiveSize_t const endRow) {
    for (std::size_t y {startRow}; y < endRow; ++y) {
        for (std::size_t x {0}; x < uint {bb.w}; ++x) {
            Color::RGBA const color {
                static_cast<u8>(Color::RGBA::maxChannelValue * (static_cast<double>(x) / static_cast<double>(uint {bb.w}))),
                    static_cast<u8>(Color::RGBA::maxChannelValue * (1 - (static_cast<double>(x) / static_cast<double>(uint {bb.w})) * (static_cast<double>(y) / static_cast<double>(uint {bb.h})))),
                    static_cast<u8>(Color::RGBA::maxChannelValue * (static_cast<double>(y) / static_cast<double>(uint {bb.h}))),
            };
            draw_solid_square(bb, {static_cast<int>(x), static_cast<int>(y), 1, 1}, color);
        }
    }
}

auto static draw_playarea_rows(BackBuffer bb, PositiveSize_t const startRow, PositiveSize_t const endRow, Board const& board, PositiveUInt const positiveScale) {
    uint const scale {positiveScale};
    for (std::size_t y {startRow}; y < endRow; ++y) {
        for (std::size_t x {0}; x < Board::columns; ++x) {
            auto currindex {y * Board::columns + x};
            auto const& block {board.data[currindex]};
            auto color {block.isActive ? block.color : Color::black};
            Rect<int> square {
                static_cast<int>((x + gPlayAreaDim.x) * scale),
                    static_cast<int>((y - 2 + gPlayAreaDim.y) * scale),
                    static_cast<int>(scale),
                    static_cast<int>(scale)
            };
            draw_solid_square(bb, square, color);
        }
    }
}

auto draw(ProgramState& programState, GameState& gameState) -> void {
    auto bb {get_back_buffer()};
    auto const scale {get_window_scale()};
    auto const threadCount {std::thread::hardware_concurrency()};
    std::vector<std::thread> threads {};
    threads.reserve(threadCount);

    // draw window background
    {
        auto const rowsPerThread {uint {bb.h} / threadCount};
        for (std::size_t i {0}; i < threadCount - 1; ++i) {
            threads.emplace_back(&draw_background_rows, bb, PositiveSize_t {rowsPerThread * i}, PositiveSize_t {rowsPerThread * (i + 1)});
        }
        threads.emplace_back(&draw_background_rows, bb, PositiveSize_t {rowsPerThread * (threadCount - 1)}, bb.h);

        for (auto& thread : threads) {
            thread.join();
        }
        threads.clear();
    }

    switch (programState.levelType) {
        case ProgramState::LevelType::Menu: {
        } break;
        case ProgramState::LevelType::Game: {
            // draw playarea
            {
                auto const positiveScale {PositiveUInt {scale}};
                auto const rowsPerThread {(Board::rows - 2) / threadCount};
                for (std::size_t i {0}; i < threadCount - 1; ++i) {
                    threads.emplace_back(&draw_playarea_rows, bb, PositiveSize_t {2 + (rowsPerThread * i)}, PositiveSize_t {2 + rowsPerThread * (i + 1)}, gameState.board, positiveScale);
                }
                threads.emplace_back(&draw_playarea_rows, bb, PositiveSize_t {2 + rowsPerThread * (threadCount - 1)}, PositiveSize_t {Board::rows}, gameState.board, positiveScale);

                for (auto& thread : threads) {
                    thread.join();
                }
                threads.clear();
            }

            // draw currentShape and its shadow
            auto draw_shape_in_play_area = [&](Shape& shape) {
                for (auto& position : shape.get_absolute_block_positions()) {
                    // since the top 2 rows shouldn't be visible, the y
                    // position for drawing is 2 less than the shape's
                    auto const actualYPosition {position.y - 2};

                    // don't draw if square is above the playarea
                    if (actualYPosition + gPlayAreaDim.y < gPlayAreaDim.y) { continue; }
                    Rect<int> square {
                        (position.x + gPlayAreaDim.x) * scale,
                            (actualYPosition + gPlayAreaDim.y) * scale,
                            scale,
                            scale
                    };

                    draw_solid_square(bb, square, shape.color);
                }
            };

            draw_shape_in_play_area(gameState.currentShapeShadow);
            draw_shape_in_play_area(gameState.currentShape);

            // draw shape previews
            auto const previewArray {gameState.shapePool.get_preview_shapes_array()};
            auto i {0};
            for (auto const* shapePointer : previewArray) {
                auto shape {*shapePointer};
                shape.pos.x = gSidebarDim.x;
                auto const ySpacing {3}; // max height of a shape is 2 + 1 for a block of space
                shape.pos.y = gSidebarDim.y + ySpacing * i;
                for (auto& position : shape.get_absolute_block_positions()) {
                    Rect<int> square {
                        (position.x) * scale,
                            (position.y) * scale,
                            scale,
                            scale
                    };
                    draw_solid_square(bb, square, shape.color);
                }
                ++i;
            }

            // draw held shape
            Rect<int> holdShapeDim{};
            holdShapeDim.x = gHoldShapeDim.x * scale;
            holdShapeDim.y = gHoldShapeDim.y * scale;
            holdShapeDim.w = gHoldShapeDim.w * scale;
            holdShapeDim.h = gHoldShapeDim.h * scale;
            draw_solid_square(bb, holdShapeDim, Color::black);
            if (gameState.holdShapeType) {
                Shape shape {*gameState.holdShapeType};
                shape.pos.x = 0;
                shape.pos.y = 0;

                auto is_even = [](auto const n) { return (n % 2) == 0; };
                // offset to center shape inside hold square
                auto const shapeDimensions {Shape::dimensions[static_cast<std::size_t>(shape.type)]};
                auto const xOffset {is_even(gHoldShapeDim.w - shapeDimensions.w) ? 1.0 : 0.5};
                auto const yOffset {is_even(gHoldShapeDim.h - shapeDimensions.h) ? 0.0 : 0.5};

                for (auto& position : shape.get_absolute_block_positions()) {
                    Rect<int> square {
                        static_cast<int>((position.x + gHoldShapeDim.x + xOffset) * scale),
                            static_cast<int>((position.y + gHoldShapeDim.y + yOffset) * scale),
                            scale,
                            scale
                    };
                    draw_solid_square(bb, square, shape.color);
                }
            }
        } break;
    }

    UI::draw(bb);

    swap_buffer();
}
