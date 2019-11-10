#include <cassert>
#include <ctime>
#include <iostream>
#include <array>
#include <string_view>
#include <random>
#include <numeric>
#include <string>
#include <vector>
#include <optional>

#include "jint.h"
#include "platform.hpp"
#include "shape.hpp"
#include "board.hpp"
#include "util.hpp"
#include "draw.hpp"
#include "tests.hpp"

#include "core.hpp"

using namespace std::string_literals;

auto gRunning = true;

auto currentclock = (decltype(clock())) 0;
auto dropclock = (decltype(clock())) 0;
auto constexpr lockdelay = (decltype(clock())) CLOCKS_PER_SEC / 2;
auto lockclock = (decltype(clock())) 0;

auto dropSpeed = 1.0;
auto maxDropSpeed = 0.1;

auto run() -> void
{
    tests::run();
    gRunning = true;

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

    ShapePool shapePool{shapes};

    auto currentShape = shapePool.current_shape();
    auto currentShapeShadow = currentShape.get_shadow(board);

    std::optional<Shape> holdShape{};
    auto hasHeld = false;

    auto hiScore = 0;

    auto linesCleared = 0;
    auto level = 1;
    auto score = 0;

    auto init = [&] {
        linesCleared = 0;
        level = 1;
        score = 0;
        currentclock = clock();
        dropclock = currentclock;
        lockclock = currentclock;
        srand(time(NULL));
    };

    init();

    auto reset_game = [&] {
        init();

        board = {};
        shapePool.reshuffle();
        currentShape = shapePool.current_shape();
        currentShapeShadow = currentShape.get_shadow(board);
    };

    enum class GameState {
        MENU,
        GAME,
    };
    auto gameState = GameState::MENU;

    struct Button {
        Squaref dimensions;
        FontString text;
    };

    struct Menu {
        enum class ID {
            MAIN,
        };
        ID id;
        std::vector<Button> buttons;
    };

    // FIXME: The font string doesn't get updated if the window changes size
    auto windim = get_window_dimensions();
    auto playButtonFontString = FontString::from_height_normalized("PLAY", 1.f / 10.f);
    auto x = (1.f - playButtonFontString.normalizedW) / 2;
    auto y = 2.f / 10.f;
    auto playButton = Button {
        Squaref {
            x, y,
            playButtonFontString.normalizedW, playButtonFontString.normalizedH
        },
        std::move(playButtonFontString)
    };

    Menu mainMenu = {
        Menu::ID::MAIN,
    };
    mainMenu.buttons.push_back(std::move(playButton));

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
                reset_game();
            } else if (message.type == Message::Type::INCREASE_WINDOW_SIZE) {
                change_window_scale(get_window_scale() + 1);
            } else if (message.type == Message::Type::DECREASE_WINDOW_SIZE) {
                change_window_scale(get_window_scale() - 1);
            } else if (gameState == GameState::GAME) {

                auto update_shadow_and_clocks = [&](bool isGrounded) {
                    currentShapeShadow = currentShape.get_shadow(board);
                    lockclock = currentclock;
                    if (isGrounded) {
                        dropclock = currentclock;
                    }
                };

                if (message.type == Message::Type::MOVE_RIGHT) {
                    // if currentShape is on top of a block before move,
                    // the drop clock needs to be reset
                    auto isGrounded = !board.is_valid_move(currentShape, {0, 1});
                    if (currentShape.try_move(board, {1, 0})) {
                        update_shadow_and_clocks(isGrounded);
                    }
                } else if (message.type == Message::Type::MOVE_LEFT) {
                    // if currentShape is on top of a block before move,
                    // the drop clock needs to be reset
                    auto isGrounded = !board.is_valid_move(currentShape, {0, 1});
                    if (currentShape.try_move(board, {-1, 0})) {
                        update_shadow_and_clocks(isGrounded);
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
                        update_shadow_and_clocks(isGrounded);
                    }
                } else if (message.type == Message::Type::ROTATE_RIGHT) {
                    // if currentShape is on top of a block before rotation,
                    // the drop clock needs to be reset
                    auto isGrounded = !board.is_valid_move(currentShape, {0, 1});
                    if (currentShape.rotate(board, Shape::Rotation::RIGHT)) {
                        update_shadow_and_clocks(isGrounded);
                    }
                } else if (message.type == Message::Type::HOLD) {
                    if (!hasHeld) {
                        hasHeld = true;
                        if (holdShape) {
                            auto tmp = holdShape;

                            holdShape = Shape(currentShape.type, board);
                            currentShape = Shape(tmp->type, board);
                        } else {
                            holdShape = Shape(currentShape.type, board);
                            currentShape = shapePool.next_shape();
                        }

                        auto isGrounded = !board.is_valid_move(currentShape, {0, 1});
                        update_shadow_and_clocks(isGrounded);
                    }
                }
            } else if (gameState == GameState::MENU) {
                if (message.type == Message::Type::MOUSEBUTTONDOWN) {
                    if (!currentMenu) {
                        std::cerr << "ERROR: currentMenu is null, but gameState is GameState::MENU\n";
                    }

                    if (currentMenu->id == Menu::ID::MAIN) {
                        auto& playButton = currentMenu->buttons[0];
                        auto screenSpaceDimensions = to_screen_space(playButton.dimensions);

                        if (message.x > screenSpaceDimensions.x &&
                            message.x < screenSpaceDimensions.x + screenSpaceDimensions.w &&
                            message.y > screenSpaceDimensions.y &&
                            message.y < screenSpaceDimensions.y + screenSpaceDimensions.h)
                        {
                            gameState = GameState::GAME;
                            reset_game();
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

                    auto rowsCleared = board.remove_full_rows();
                    linesCleared += rowsCleared;
                    if (rowsCleared == 1) {
                        score += level * 100;
                    } else if (rowsCleared == 2) {
                        score += level * 300;
                    } else if (rowsCleared == 3) {
                        score += level * 500;
                    } else if (rowsCleared == 4) {
                        score += level * 800;
                    }

                    level = linesCleared / 10 + 1;

                    currentShape = shapePool.next_shape();
                    // update shape shadow
                    currentShapeShadow = currentShape.get_shadow(board);
                    lockclock = currentclock;

                    hasHeld = false;

                    // game over if the new shape spawned on top of another shape
                    if (!board.is_valid_shape(currentShape)) {
                        gameOver = true;
                    }

                    if (gameOver) {
                        std::cout << "Game Over!\n";
                        if (score > hiScore) hiScore = score;
                        gameState = GameState::MENU;
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
                auto color = RGB {
                    int(0xff * (float(x) / windim.w)),
                    int(0xff * (1 - (float(x) / windim.w) * (float(y) / windim.h))),
                    int(0xff * (float(y) / windim.h)),
                };
                draw_solid_square(bb, {float(x), float(y), 1, 1}, color);
            }
        }

        switch (gameState) {
            case GameState::MENU: {
                auto menuLabel = FontString::from_height_normalized("MENU", 1.f / 10.f);
                auto x = (1.f - menuLabel.normalizedW) / 2.f;
                auto y = 1.f / 10.f;
                draw_font_string_normalized(bb, menuLabel, x, y);
                if (!currentMenu) {
                    std::cerr << "ERROR: currentMenu is null, but gameState is GameState::MENU\n";
                }
                for (auto& button : currentMenu->buttons) {
                    auto outlineScreenSpace = to_screen_space(button.dimensions);

                    draw_hollow_square(bb, outlineScreenSpace, {0, 0, 0});
                    draw_font_string(bb, button.text, outlineScreenSpace.x, outlineScreenSpace.y);
                }

                // draw high score
                using namespace std::string_literals;
                auto hiScoreString = "High Score: "s + std::to_string(hiScore);
                auto fontString = FontString::from_height_normalized(hiScoreString, 0.048);
                draw_font_string_normalized(bb, fontString, 1.0 - fontString.normalizedW - 0.01, 0.01);
            } break;
            case GameState::GAME: {
                // draw playarea background
                for (auto y = 2; y < Board::rows; ++y) {
                    for (auto x = 0; x < Board::columns; ++x) {
                        auto currindex = y * Board::columns + x;
                        auto& block = board.data[currindex];
                        auto color = block.isActive ? block.color : RGB { 0, 0, 0 };
                        auto square = Squaref {
                            float((x + gPlayAreaDim.x) * scale),
                            float((y - 2 + gPlayAreaDim.y) * scale),
                            float(scale),
                            float(scale)
                        };
                        draw_solid_square(bb, square, color);
                    }
                }

                auto draw_shape_in_play_area = [&](Shape& shape, int transparency) {
                    for (auto& position : shape.get_absolute_block_positions()) {
                        // since the top 2 rows shouldn't be visible, the y
                        // position for drawing is 2 less than the shape's
                        auto actualYPosition = position.y - 2;

                        // don't draw if square is above the playarea
                        if (actualYPosition + gPlayAreaDim.y < gPlayAreaDim.y) continue;
                        auto square = Squaref {
                            float((position.x + gPlayAreaDim.x) * scale),
                            float((actualYPosition + gPlayAreaDim.y) * scale),
                            float(scale),
                            float(scale)
                        };

                        draw_solid_square(bb, square, shape.color, transparency);
                    }
                };

                draw_shape_in_play_area(currentShapeShadow, 0xff / 2);
                draw_shape_in_play_area(currentShape, 0xff);

                // draw shape previews
                auto previewArray = shapePool.get_preview_shapes_array();
                auto i = 0;
                for (auto shapePointer : previewArray) {
                    auto shape = *shapePointer;
                    shape.pos.x = gSidebarDim.x;
                    auto const ySpacing = 3; // max height of a shape is 2 + 1 for a block of space
                    shape.pos.y = gSidebarDim.y + ySpacing * i;
                    for (auto& position : shape.get_absolute_block_positions()) {
                        auto square = Squaref {
                            float((position.x) * scale),
                            float((position.y) * scale),
                            float(scale),
                            float(scale)
                        };
                        draw_solid_square(bb, square, shape.color);
                    }
                    ++i;
                }

                // draw score
                auto scoreString = "Score: "s + std::to_string(score);
                auto scoreFontString = FontString::from_height_normalized(scoreString, 0.048);
                draw_font_string_normalized(bb, scoreFontString, 1.0 - scoreFontString.normalizedW - 0.01, 0.01);

                // draw level
                auto levelString = "Level: "s + std::to_string(level) + " (" + std::to_string(linesCleared) + "/" + std::to_string(level * 10) + ")";
                auto levelFontString = FontString::from_height_normalized(levelString, 0.048);
                draw_font_string_normalized(bb, levelFontString, 1.0 - levelFontString.normalizedW - 0.01, 0.01 + levelFontString.normalizedH);

                // draw held shape
                Squaref holdShapeDim{};
                holdShapeDim.x = gHoldShapeDim.x * scale;
                holdShapeDim.y = gHoldShapeDim.y * scale;
                holdShapeDim.w = gHoldShapeDim.w * scale;
                holdShapeDim.h = gHoldShapeDim.h * scale;
                draw_solid_square(bb, holdShapeDim, {0, 0, 0});
                if (holdShape) {
                    auto shape = *holdShape;
                    shape.pos.x = 0;
                    shape.pos.y = 0;

                    auto is_even = [](auto n) { return (n % 2) == 0; };
                    // offset to center shape inside hold square
                    auto shapeDimensions = Shape::dimensions[int(shape.type)];
                    auto xOffset = is_even(gHoldShapeDim.w - shapeDimensions.w) ? 1.0f : 0.5f;
                    auto yOffset = is_even(gHoldShapeDim.h - shapeDimensions.h) ? 0.0f : 0.5f;

                    for (auto& position : shape.get_absolute_block_positions()) {
                        auto square = Squaref {
                            float((position.x + gHoldShapeDim.x + xOffset) * scale),
                            float((position.y + gHoldShapeDim.y + yOffset) * scale),
                            float(scale),
                            float(scale)
                        };
                        draw_solid_square(bb, square, shape.color);
                    }
                }
            } break;
            default: {
                assert(false);
            } break;
        }

        swap_buffer();
    }
}
