#include <cassert>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <array>
#include <string_view>
#include <random>

#include <SDL2/SDL.h>

#include "jint.h"
#include "core.hpp"
#include "platform.hpp"
#include "shape.hpp"
#include "board.hpp"

#include "util.hpp"

auto gRunning = true;

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

struct Point {
    float x;
    float y;
};

auto alpha_blend_channel(int bg, int fg, int alpha) -> float
{
    assert(bg >= 0 && bg <= 255);
    assert(fg >= 0 && fg <= 255);
    assert(alpha >= 0 && alpha <= 255);

    auto alphaRatio = alpha / 255.f;
    return fg * alphaRatio + bg * (1 - alphaRatio);
}

auto draw_font_character(BackBuffer& buf, FontCharacter& fontCharacter, int realX, int realY)
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

auto draw_font_string(BackBuffer& buf, FontString& fontString, int x, int y)
{
    for (auto& fontCharacter : fontString.data) {
        draw_font_character(buf, fontCharacter, x, y);
        x += fontCharacter.advance;
    }
}

auto draw_font_string_normalized(BackBuffer& buf, FontString& fontString, float x, float y)
{
    x *= buf.w;
    y *= buf.h;

    draw_font_string(buf, fontString, x, y);
}

auto draw_text(BackBuffer& buf, std::string_view text, int x, int y, float pixelHeight)
{
    auto fontString = FontString(text, pixelHeight);
    draw_font_string(buf, fontString, x, y);
}

auto draw_text_normalized(BackBuffer& buf, std::string_view text, float x, float y, float pixelHeight)
{
    x *= buf.w;
    y *= buf.h;
    pixelHeight *= buf.h;

    draw_text(buf, text, x, y, pixelHeight);
}

auto draw_solid_square(BackBuffer& buf, Square sqr, uint r, uint g, uint b, uint a = 0xff)
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

            *currbyte = alpha_blend_channel(*currbyte, b, a);
            ++currbyte;
            *currbyte = alpha_blend_channel(*currbyte, g, a);
            ++currbyte;
            *currbyte = alpha_blend_channel(*currbyte, r, a);
        }
    }
}

auto draw_solid_square_normalized(BackBuffer& buf, Square sqr, uint r, uint g, uint b, uint a = 0xff)
{
    sqr.x *= buf.w;
    sqr.y *= buf.h;
    sqr.w *= buf.w;
    sqr.h *= buf.h;

    draw_solid_square(buf, sqr, r, g, b, a);
}

auto draw_hollow_square(BackBuffer& buf, Square sqr, int r, int g, int b, int a = 0xff, int borderSize = 1)
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

            *currbyte = alpha_blend(*currbyte, b, a);
            ++currbyte;
            *currbyte = alpha_blend(*currbyte, g, a);
            ++currbyte;
            *currbyte = alpha_blend(*currbyte, r, a);
        }
    }
}

auto draw_hollow_square_normalized(BackBuffer& buf, Square sqr, int r, int g, int b, int a = 0xff, int borderSize = 1)
{
    sqr.x *= buf.w;
    sqr.y *= buf.h;
    sqr.w *= buf.w;
    sqr.h *= buf.h;

    draw_hollow_square(buf, sqr, r, g, b, a, borderSize);
}

auto draw_image(BackBuffer& backBuf, Point dest, BackBuffer& img)
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

auto currentclock = (decltype(clock())) 0;
auto dropclock = (decltype(clock())) 0;
auto constexpr lockdelay = (decltype(clock())) CLOCKS_PER_SEC / 2;
auto lockclock = (decltype(clock())) 0;

auto highScore = 0;

auto dropSpeed = 1.0;
auto maxDropSpeed = 0.1;

auto init()
{
    currentclock = clock();
    dropclock = currentclock;
    lockclock = currentclock;

    srand(time(NULL));
}

namespace tests {
auto remove_full_rows() {
    auto y = Block { {}, true };
    auto n = Block { {}, false };
    Board boardStart;
    boardStart.data = {
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, y, y, y, n, n, n,
        n, n, n, y, y, n, y, n, n, n,
        n, y, y, y, y, n, n, n, n, n,
        n, y, n, n, y, n, n, n, n, n,
        y, y, y, y, y, y, y, y, y, y,
        y, n, y, n, y, n, y, n, y, n,
        n, y, n, y, n, y, n, y, n, y,
        y, y, y, y, y, y, y, y, y, y,
    };

    Board boardEnd;
    boardEnd.data = {
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, y, y, y, n, n, n,
        n, n, n, y, y, n, y, n, n, n,
        n, y, y, y, y, n, n, n, n, n,
        n, y, n, n, y, n, n, n, n, n,
        y, n, y, n, y, n, y, n, y, n,
        n, y, n, y, n, y, n, y, n, y,
    };

    boardStart.remove_full_rows();

    for (auto i = 0ul; i < boardStart.data.size(); ++i) {
        assert(boardStart.data[i].isActive == boardEnd.data[i].isActive);
    }
}

auto run() {
    remove_full_rows();
}
} // namespace test

auto run() -> void
{
    tests::run();
    gRunning = true;

    init();

    Board board = {};

    std::array<Shape, 7> const shapes = {
        Shape(Shape::Type::I, board),
        Shape(Shape::Type::L, board),
        Shape(Shape::Type::J, board),
        Shape(Shape::Type::O, board),
        Shape(Shape::Type::S, board),
        Shape(Shape::Type::Z, board),
        Shape(Shape::Type::T, board),
    };

    class ShapePool {
        std::array<const Shape*, 7> shapePool;
        decltype(shapePool) previewPool;
        decltype(shapePool.begin()) currentShapeIterator;

    public:
        ShapePool(const std::array<Shape, 7>& shapes) {
            shapePool = {
                &shapes[0], &shapes[1], &shapes[2],
                &shapes[3], &shapes[4], &shapes[5],
                &shapes[6],
            };
            previewPool = shapePool;

            // TODO: seed random engine
            std::shuffle(shapePool.begin(), shapePool.end(), std::default_random_engine());
            std::shuffle(previewPool.begin(), previewPool.end(), std::default_random_engine());
            currentShapeIterator = shapePool.begin();
        }

        auto next_shape() -> Shape {
            ++currentShapeIterator;
            if (currentShapeIterator == shapePool.end()) {
                shapePool = previewPool;
                currentShapeIterator = shapePool.begin();
                std::shuffle(previewPool.begin(), previewPool.end(), std::default_random_engine());
            }
            return **currentShapeIterator;
        }

        auto current_shape() -> Shape {
            return **currentShapeIterator;
        }

        auto get_preview_shapes_array() -> ArrayStack<Shape const*, 14> {
            ArrayStack<Shape const*, 14> lookaheadArray = {};
            for (auto it = currentShapeIterator + 1; it != shapePool.end(); ++it) {
                lookaheadArray.push_back(*it);
            }
            for (auto it = previewPool.cbegin(); it != previewPool.cend(); ++it) {
                lookaheadArray.push_back(*it);
            }
            return lookaheadArray;
        }
    };
    ShapePool shapePool{shapes};

    auto calculateShapeShadow = [&board](Shape& shape) {
        auto shapeShadow = shape;
        while (board.is_valid_move(shapeShadow, {0, 1})) {
            ++shapeShadow.pos.y;
        }
        return shapeShadow;
    };

    auto currentShape = shapePool.current_shape();
    auto currentShapeShadow = calculateShapeShadow(currentShape);

    enum class GameState {
        MENU,
        GAME,
    };
    auto gameState = GameState::MENU;

    struct Button {
        Square dimensions;
        std::string text;
    };

    struct Menu {
        enum class ID {
            MAIN,
        };
        ID id;
        std::vector<Button> buttons;
    };

    auto windim = get_window_dimensions();
    auto playButton = Button {
        Square {
            (1.f / 3.f), (2.f / 10.f),
            // FIXME: w doesn't know anything about the length of the font text
            //        at this point.
            (1.f / 3.f), (1.f / 10.f)
        },
        std::string("PLAY")
    };

    auto mainMenu = Menu {
        Menu::ID::MAIN,
        { playButton }
    };

    auto currentMenu = &mainMenu;

    while (gRunning) {
        auto newclock = clock();
        auto frameclocktime = newclock - currentclock;
        currentclock = newclock;

        // delta = (double)frameclocktime / CLOCKS_PER_SEC;
        /* auto framemstime = 1000.0 * delta; */

        // TODO: sleep so cpu doesn't melt

        // input
        Message message;
        while ((message = handle_input()).type != Message::Type::NONE) {
            // First check messages independent of whether in menu or game
            if (message.type == Message::Type::QUIT) {
                gRunning = false;
            } else if (message.type == Message::Type::RESET) {
                /* init(); */
            } else if (message.type == Message::Type::INCREASE_WINDOW_SIZE) {
                change_window_scale(get_window_scale() + 1);
            } else if (message.type == Message::Type::DECREASE_WINDOW_SIZE) {
                change_window_scale(get_window_scale() - 1);
            } else if (gameState == GameState::GAME) {
                if (message.type == Message::Type::MOVE_RIGHT) {
                    // if currentShape is on top of a block before move,
                    // the drop clock needs to be reset
                    auto isGrounded = !board.is_valid_move(currentShape, {0, 1});
                    if (currentShape.try_move(board, {1, 0})) {
                        currentShapeShadow = calculateShapeShadow(currentShape);
                        lockclock = currentclock;
                        if (isGrounded) {
                            dropclock = currentclock;
                        }
                    }
                } else if (message.type == Message::Type::MOVE_LEFT) {
                    // if currentShape is on top of a block before move,
                    // the drop clock needs to be reset
                    auto isGrounded = !board.is_valid_move(currentShape, {0, 1});
                    if (currentShape.try_move(board, {-1, 0})) {
                        currentShapeShadow = calculateShapeShadow(currentShape);
                        lockclock = currentclock;
                        if (isGrounded) {
                            dropclock = currentclock;
                        }
                    }
                } else if (message.type == Message::Type::INCREASE_SPEED) {
                    dropSpeed = maxDropSpeed;
                } else if (message.type == Message::Type::RESET_SPEED) {
                    dropSpeed = 1.0;
                } else if (message.type == Message::Type::DROP) {
                    while (currentShape.try_move(board, {0, 1})) {
                        lockclock = currentclock;
                    }
                } else if (message.type == Message::Type::ROTATE_LEFT) {
                    // if currentShape is on top of a block before rotation,
                    // the drop clock needs to be reset
                    auto isGrounded = !board.is_valid_move(currentShape, {0, 1});
                    if (currentShape.rotate(board, Shape::Rotation::LEFT)) {
                        currentShapeShadow = calculateShapeShadow(currentShape);
                        lockclock = currentclock;
                        if (isGrounded) {
                            dropclock = currentclock;
                        }
                    }
                } else if (message.type == Message::Type::ROTATE_RIGHT) {
                    // if currentShape is on top of a block before rotation,
                    // the drop clock needs to be reset
                    auto isGrounded = !board.is_valid_move(currentShape, {0, 1});
                    if (currentShape.rotate(board, Shape::Rotation::RIGHT)) {
                        currentShapeShadow = calculateShapeShadow(currentShape);
                        lockclock = currentclock;
                        if (isGrounded) {
                            dropclock = currentclock;
                        }
                    }
                }
            } else if (gameState == GameState::MENU) {
                if (message.type == Message::Type::MOUSEBUTTONDOWN) {
                    if (!currentMenu) {
                        std::cerr << "ERROR: currentMenu is null, but gameState is GameState::MENU\n";
                    }

                    if (currentMenu->id == Menu::ID::MAIN) {
                        auto& playButton = currentMenu->buttons[0];
                        auto screenSpaceDimensions = playButton.dimensions.to_screen_space();

                        if (message.x > screenSpaceDimensions.x &&
                            message.x < screenSpaceDimensions.x + screenSpaceDimensions.w &&
                            message.y > screenSpaceDimensions.y &&
                            message.y < screenSpaceDimensions.y + screenSpaceDimensions.h)
                        {
                            gameState = GameState::GAME;
                            init();
                        }
                    }
                }
            }
        }

        // sim
        if (gameState == GameState::GAME) {
            // 1 drop per second
            auto nextdropclock = dropclock + dropSpeed * CLOCKS_PER_SEC;
            if (currentclock > nextdropclock) {
                dropclock = currentclock;
                if (currentShape.try_move(board, {0, 1})) {
                    lockclock = currentclock;
                }
            }

            if (currentclock > lockclock + lockdelay) {
                // only care about locking if currentShape is on top of a block
                if (!board.is_valid_move(currentShape, {0, 1})) {
                    // game over if entire piece is above visible portion
                    // of board
                    auto gameOver = true;
                    for (auto pos : currentShape.get_absolute_block_positions()) {
                        if (pos.y > 1) {
                            gameOver = false;
                        }
                    }

                    // fix currentBlocks position on board
                    for (auto position : currentShape.get_absolute_block_positions()) {
                        assert(board.is_valid_spot(position));
                        auto boardIndex = position.y * board.columns + position.x;
                        board.data[boardIndex] = {currentShape.color, true};
                    }

                    board.remove_full_rows();
                    currentShape = shapePool.next_shape();
                    // update shape shadow
                    currentShapeShadow = calculateShapeShadow(currentShape);
                    lockclock = currentclock;

                    // game over if the new shape spawned on top of another shape
                    if (!board.is_valid_shape(currentShape)) {
                        gameOver = true;
                    }

                    if (gameOver) {
                        std::cout << "Game Over!\n";
                        gRunning = false;
                    }
                }
            }
        }

        // draw border
        auto windim = get_window_dimensions();
        auto bb = get_back_buffer();
        auto scale = get_window_scale();
        for (auto y = 0; y < windim.h; ++y) {
            for (auto x = 0; x < windim.w; ++x) {
                draw_solid_square(bb, {float(x), float(y), 1, 1}, 0xff * (float(x) / windim.w), 0xff * (1 - (float(x) / windim.w) * (float(y) / windim.h)), 0xff * (float(y) / windim.h));
            }
        }

        switch (gameState) {
            case GameState::MENU: {
                draw_text_normalized(bb, "MENU", (1.f / 3.f), (1.f / 10.f), (1.f / 10.f));
                if (!currentMenu) {
                    std::cerr << "ERROR: currentMenu is null, but gameState is GameState::MENU\n";
                }
                for (auto& button : currentMenu->buttons) {
                    auto outlineScreenSpace = button.dimensions.to_screen_space();

                    auto fontString = FontString(button.text, outlineScreenSpace.h);
                    auto textWidth = std::accumulate(fontString.data.begin(), fontString.data.end(), 0.f,
                                                     [](float const& a, FontCharacter const& b) {
                        return a + b.advance;
                    });

                    outlineScreenSpace.w = textWidth;
                    draw_hollow_square(bb, outlineScreenSpace, 0, 0, 0);
                    draw_font_string(bb, fontString, outlineScreenSpace.x, outlineScreenSpace.y);
                }
            } break;
            case GameState::GAME: {
                // draw background
                for (auto y = 2; y < board.rows; ++y) {
                    for (auto x = 0; x < board.columns; ++x) {
                        auto currindex = y * board.columns + x;
                        auto& block = board.data[currindex];
                        auto color = block.isActive ? block.color : Color { 0, 0, 0 };
                        draw_solid_square(bb, {float((x + 1) * scale), float((y + 1) * scale), float(scale), float(scale)}, color.r, color.g, color.b);
                    }
                }

                // draw shadow
                for (auto& position : currentShapeShadow.get_absolute_block_positions()) {
                    draw_solid_square(bb, {float((position.x + 1) * float(scale)), float((position.y + 1) * float(scale)), float(scale), float(scale)}, currentShapeShadow.color.r, currentShapeShadow.color.g, currentShapeShadow.color.b, 0xff / 2);
                }

                // draw current shape
                for (auto& position : currentShape.get_absolute_block_positions()) {
                    draw_solid_square(bb, {float((position.x + 1) * float(scale)), float((position.y + 1) * float(scale)), float(scale), float(scale)}, currentShape.color.r, currentShape.color.g, currentShape.color.b);
                }

                // draw shape previews
                auto previewArray = shapePool.get_preview_shapes_array();
                auto i = 0;
                for (auto shapePointer : previewArray) {
                    auto shape = *shapePointer;
                    shape.pos.x = baseWindowWidth - 6;
                    shape.pos.y += 3 + 3 * i;
                    for (auto& position : shape.get_absolute_block_positions()) {
                        draw_solid_square(bb, {float((position.x + 1) * float(scale)), float((position.y + 1) * float(scale)), float(scale), float(scale)}, shape.color.r, shape.color.g, shape.color.b);
                    }
                    ++i;
                }

                // cover top part of border
                auto topSize = scale * 3;
                for (auto y = 0; y < topSize; ++y) {
                    for (auto x = 0; x < windim.w; ++x) {
                        draw_solid_square(bb, {float(x), float(y), 1, 1}, 0xff * (float(x) / windim.w), 0xff * (1 - (float(x) / windim.w) * (float(y) / windim.h)), 0xff * (float(y) / windim.h));
                    }
                }

                draw_text(bb, "Hello World!", 10, 10, 48);
            } break;
            default: {
                assert(false);
            } break;
        }

        swap_buffer();
    }
}
