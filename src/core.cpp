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

    auto linesCleared = 0;

    auto init = [&] {
        linesCleared = 0;
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
        Square dimensions;
        FontString text;
    };

    struct Menu {
        enum class ID {
            MAIN,
        };
        ID id;
        std::vector<Button> buttons;
    };

    auto windim = get_window_dimensions();
    auto playButtonFontString = FontString::from_height_normalized("PLAY", 1.f / 10.f);
    auto playButton = Button {
        Square {
            (1.f / 3.f), (2.f / 10.f),
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
                        auto screenSpaceDimensions = playButton.dimensions.to_screen_space();

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
                draw_text_normalized(bb, "MENU", (1.f / 3.f), (1.f / 10.f), (1.f / 10.f));
                if (!currentMenu) {
                    std::cerr << "ERROR: currentMenu is null, but gameState is GameState::MENU\n";
                }
                for (auto& button : currentMenu->buttons) {
                    auto outlineScreenSpace = button.dimensions.to_screen_space();

                    draw_hollow_square(bb, outlineScreenSpace, {0, 0, 0});
                    draw_font_string(bb, button.text, outlineScreenSpace.x, outlineScreenSpace.y);
                }
            } break;
            case GameState::GAME: {
                // draw background
                for (auto y = 2; y < board.rows; ++y) {
                    for (auto x = 0; x < board.columns; ++x) {
                        auto currindex = y * board.columns + x;
                        auto& block = board.data[currindex];
                        auto color = block.isActive ? block.color : RGB { 0, 0, 0 };
                        auto square = Square {
                            float((x + 1) * scale),
                            float((y + 1) * scale),
                            float(scale),
                            float(scale)
                        };
                        draw_solid_square(bb, square, color);
                    }
                }

                // draw shadow
                for (auto& position : currentShapeShadow.get_absolute_block_positions()) {
                    auto square = Square {
                        float((position.x + 1) * float(scale)),
                        float((position.y + 1) * float(scale)),
                        float(scale),
                        float(scale)
                    };
                    draw_solid_square(bb, square, currentShapeShadow.color, 0xff / 2);
                }

                // draw current shape
                for (auto& position : currentShape.get_absolute_block_positions()) {
                    auto square = Square {
                        float((position.x + 1) * float(scale)),
                        float((position.y + 1) * float(scale)),
                        float(scale),
                        float(scale)
                    };
                    draw_solid_square(bb, square, currentShape.color);
                }

                // draw shape previews
                auto previewArray = shapePool.get_preview_shapes_array();
                auto i = 0;
                for (auto shapePointer : previewArray) {
                    auto shape = *shapePointer;
                    shape.pos.x = baseWindowWidth - 6;
                    shape.pos.y += 3 + 3 * i;
                    for (auto& position : shape.get_absolute_block_positions()) {
                        auto square = Square {
                            float((position.x + 1) * float(scale)),
                            float((position.y + 1) * float(scale)),
                            float(scale),
                            float(scale)
                        };
                        draw_solid_square(bb, square, shape.color);
                    }
                    ++i;
                }

                // cover top part of border
                auto topSize = scale * 3;
                for (auto y = 0; y < topSize; ++y) {
                    for (auto x = 0; x < windim.w; ++x) {
                        auto color = RGB {
                            int(0xff * (float(x) / windim.w)),
                            int(0xff * (1 - (float(x) / windim.w) * (float(y) / windim.h))),
                            int(0xff * (float(y) / windim.h)),
                        };
                        draw_solid_square(bb, {float(x), float(y), 1, 1}, color);
                    }
                }

                // draw number of lines cleared
                auto linesClearedString = std::to_string(linesCleared);
                draw_text_normalized(bb, linesClearedString, 0.01, 0.01, 0.048);

                // draw held shape
                if (holdShape) {
                    auto shape = holdShape;
                    shape->pos.x = baseWindowWidth - 6;
                    shape->pos.y = 0;
                    for (auto& position : shape->get_absolute_block_positions()) {
                        auto square = Square {
                            float((position.x + 1) * float(scale)),
                            float((position.y + 1) * float(scale)),
                            float(scale),
                            float(scale)
                        };
                        draw_solid_square(bb, square, shape->color);
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
