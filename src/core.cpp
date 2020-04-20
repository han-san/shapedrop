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
using namespace std::string_view_literals;

// scoring formula (https://harddrop.com/wiki/Scoring):
// Single:             100 x level
// Double:             300 x level
// Triple:             500 x level
// Tetris:             800 x level
// T-Spin:             400 x level
// T-Spin Single:      800 x level
// T-Spin Double:      1200 x level
// T-Spin Triple:      1600 x level
// T-Spin Mini:        100 x level
// T-Spin Mini Single: 200 x level
// T-Spin Mini Double: 1200 x level
//
// Back to Back Tetris/T-Spin: * 1.5 (for example, back to back tetris: 1200 x level)
// Combo:     50 x combo count x level
// Soft drop: 1 point per cell
// Hard drop: 2 point per cell

enum class ClearType {
    // Don't change order. clearTypeScores and get_clear_type() depends on the
    // specific order.
    NONE,
    SINGLE,
    DOUBLE,
    TRIPLE,
    TETRIS,

    TSPIN,
    TSPIN_SINGLE,
    TSPIN_DOUBLE,
    TSPIN_TRIPLE,
    TSPIN_MINI,
    TSPIN_MINI_SINGLE,
    TSPIN_MINI_DOUBLE,
};

auto constexpr clearTypeScores = std::array {
    0,    // None
    100,  // Single
    300,  // Double
    500,  // Triple
    800,  // Tetris

    400,  // T-Spin
    800,  // T-Spin Single
    1200, // T-Spin Double
    1600, // T-Spin Triple
    100,  // T-Spin Mini
    200,  // T-Spin Mini Single
    1200  // T-Spin Mini Double
};

auto get_clear_type(int const rowsCleared, std::optional<TspinType> const tspin) {
    assert(rowsCleared <= 4);
    assert(rowsCleared >= 0);
    if (!tspin) {
        return static_cast<ClearType>(rowsCleared);
    } else {
        assert(rowsCleared <= 3);
        switch (rowsCleared) {
            case 0: {
                return *tspin == TspinType::MINI ? ClearType::TSPIN_MINI : ClearType::TSPIN;
            } break;
            case 1: {
                return *tspin == TspinType::MINI ? ClearType::TSPIN_MINI_SINGLE : ClearType::TSPIN_SINGLE;
            } break;
            case 2: {
                return *tspin == TspinType::MINI ? ClearType::TSPIN_MINI_DOUBLE : ClearType::TSPIN_DOUBLE;
            } break;
            case 3: {
                // T-spin triple requires a wallkick so there is no
                // distinction between regular and mini (although it's
                // going to be represented internally as a mini).
                return ClearType::TSPIN_TRIPLE;
            } break;
        }
    }
}

auto calculate_score(ClearType const clearType, int const level) {
    return clearTypeScores[static_cast<size_t>(clearType)] * level;
}

auto calculate_score(int const rowsCleared, std::optional<TspinType> tspin, int const level) {
    auto const clearType = get_clear_type(rowsCleared, tspin);
    return calculate_score(clearType, level);
}

enum class LevelType {
    MENU,
    GAME,
};

LevelType levelType = LevelType::MENU;
size_t highScore = 0;
auto constexpr static lockDelay = time_t(CLOCKS_PER_SEC / 2);
bool running = true;

std::array<Shape, 7> const initialShapes {
    Shape::Type::I,
    Shape::Type::L,
    Shape::Type::J,
    Shape::Type::O,
    Shape::Type::S,
    Shape::Type::Z,
    Shape::Type::T,
};

enum class BackToBackType {
    TETRIS,
    TSPIN
};

// For variables which are unique to their instance of a game
// i.e. should be reset when starting a new one
struct GameState {
    size_t linesCleared = 0;
    size_t level = 1;
    size_t score = 0;
    bool hasHeld = false;
    time_t dropClock = clock();
    time_t lockClock = dropClock;
    double dropSpeed = 1.0;
    double maxDropSpeed = 0.1;
    size_t droppedRows = 0;
    std::optional<BackToBackType> backToBackType = {};
    int comboCounter = -1;

    Board board = {};
    ShapePool shapePool = {initialShapes};
    Shape currentShape = shapePool.current_shape();
    Shape currentShapeShadow = board.get_shadow(currentShape);
    std::optional<Shape::RotationType> currentRotationType = {};
    std::optional<Shape> holdShape = {};

    auto reset() {
        *this = std::move(GameState {});
    }

};

auto run() -> void
{
    tests::run();

    GameState gameState = {};

    time_t frameStartClock = clock();

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

    while (running) {
        auto newclock = clock();
        auto frameclocktime = newclock - frameStartClock;
        frameStartClock = newclock;

        // delta = (double)frameclocktime / CLOCKS_PER_SEC;
        /* auto framemstime = 1000.0 * delta; */

        // TODO: sleep so cpu doesn't melt

        // input
        Message message;
        while ((message = handle_input()).type != Message::Type::NONE) {
            // First check messages independent of whether in menu or game
            if (message.type == Message::Type::QUIT) {
                running = false;
            } else if (message.type == Message::Type::RESET) {
                gameState.reset();
            } else if (message.type == Message::Type::INCREASE_WINDOW_SIZE) {
                change_window_scale(get_window_scale() + 1);
            } else if (message.type == Message::Type::DECREASE_WINDOW_SIZE) {
                change_window_scale(get_window_scale() - 1);
            } else if (levelType == LevelType::GAME) {

                auto update_shadow_and_clocks = [&](bool isGrounded) {
                    gameState.currentShapeShadow = gameState.board.get_shadow(gameState.currentShape);
                    gameState.lockClock = frameStartClock;
                    if (isGrounded) {
                        gameState.dropClock = frameStartClock;
                    }
                };

                if (message.type == Message::Type::MOVE_RIGHT) {
                    // if currentShape is on top of a block before move,
                    // the drop clock needs to be reset
                    auto isGrounded = !gameState.board.is_valid_move(gameState.currentShape, {0, 1});
                    if (gameState.board.try_move(gameState.currentShape, {1, 0})) {
                        update_shadow_and_clocks(isGrounded);
                        // if you move the piece you cancel the drop
                        gameState.droppedRows = 0;
                    }
                } else if (message.type == Message::Type::MOVE_LEFT) {
                    // if currentShape is on top of a block before move,
                    // the drop clock needs to be reset
                    auto isGrounded = !gameState.board.is_valid_move(gameState.currentShape, {0, 1});
                    if (gameState.board.try_move(gameState.currentShape, {-1, 0})) {
                        update_shadow_and_clocks(isGrounded);
                        // if you move the piece you cancel the drop
                        gameState.droppedRows = 0;
                    }
                } else if (message.type == Message::Type::INCREASE_SPEED) {
                    gameState.dropSpeed = gameState.maxDropSpeed;
                } else if (message.type == Message::Type::RESET_SPEED) {
                    gameState.dropSpeed = 1.0;
                } else if (message.type == Message::Type::DROP) {
                    auto droppedRows = 0;
                    while (gameState.board.try_move(gameState.currentShape, {0, 1})) {
                        gameState.lockClock = frameStartClock;
                        gameState.currentRotationType = {};
                        ++droppedRows;
                    }
                    gameState.droppedRows = droppedRows;
                } else if (message.type == Message::Type::ROTATE_LEFT) {
                    // if currentShape is on top of a block before rotation,
                    // the drop clock needs to be reset
                    auto isGrounded = !gameState.board.is_valid_move(gameState.currentShape, {0, 1});
                    if (auto const rotation = gameState.board.rotate_shape(gameState.currentShape, Shape::Rotation::LEFT); rotation) {
                        update_shadow_and_clocks(isGrounded);
                        gameState.currentRotationType = rotation;
                        // if you move the piece you cancel the drop
                        gameState.droppedRows = 0;
                    }
                } else if (message.type == Message::Type::ROTATE_RIGHT) {
                    // if currentShape is on top of a block before rotation,
                    // the drop clock needs to be reset
                    auto isGrounded = !gameState.board.is_valid_move(gameState.currentShape, {0, 1});
                    if (auto const rotation = gameState.board.rotate_shape(gameState.currentShape, Shape::Rotation::RIGHT); rotation) {
                        update_shadow_and_clocks(isGrounded);
                        gameState.currentRotationType = rotation;
                        // if you move the piece you cancel the drop
                        gameState.droppedRows = 0;
                    }
                } else if (message.type == Message::Type::HOLD) {
                    if (!gameState.hasHeld) {
                        gameState.hasHeld = true;
                        gameState.currentRotationType = {};
                        if (gameState.holdShape) {
                            auto tmp = gameState.holdShape;

                            gameState.holdShape = Shape(gameState.currentShape.type);
                            gameState.currentShape = Shape(tmp->type);
                        } else {
                            gameState.holdShape = Shape(gameState.currentShape.type);
                            gameState.currentShape = gameState.shapePool.next_shape();
                        }

                        auto isGrounded = !gameState.board.is_valid_move(gameState.currentShape, {0, 1});
                        update_shadow_and_clocks(isGrounded);
                    }
                }
            } else if (levelType == LevelType::MENU) {
                if (message.type == Message::Type::MOUSEBUTTONDOWN) {
                    if (!currentMenu) {
                        std::cerr << "ERROR: currentMenu is null, but levelType is LevelType::MENU\n";
                    }

                    if (currentMenu->id == Menu::ID::MAIN) {
                        auto& playButton = currentMenu->buttons[0];
                        auto screenSpaceDimensions = to_screen_space(playButton.dimensions);

                        if (message.x > screenSpaceDimensions.x &&
                            message.x < screenSpaceDimensions.x + screenSpaceDimensions.w &&
                            message.y > screenSpaceDimensions.y &&
                            message.y < screenSpaceDimensions.y + screenSpaceDimensions.h)
                        {
                            levelType = LevelType::GAME;
                            gameState.reset();
                        }
                    }
                }
            }
        }

        // sim
        if (levelType == LevelType::GAME) {
            // 1 drop per second
            auto nextdropClock = gameState.dropClock + gameState.dropSpeed * CLOCKS_PER_SEC;
            if (frameStartClock > nextdropClock) {
                gameState.dropClock = frameStartClock;
                if (gameState.board.try_move(gameState.currentShape, {0, 1})) {
                    gameState.lockClock = frameStartClock;
                    gameState.currentRotationType = {};
                }
            }

            if (frameStartClock > gameState.lockClock + lockDelay) {
                // only care about locking if currentShape is on top of a block
                if (!gameState.board.is_valid_move(gameState.currentShape, {0, 1})) {
                    // game over if entire piece is above visible portion
                    // of board
                    auto gameOver = true;
                    for (auto pos : gameState.currentShape.get_absolute_block_positions()) {
                        if (pos.y > 1) {
                            gameOver = false;
                        }
                    }

                    // fix currentBlocks position on board
                    for (auto position : gameState.currentShape.get_absolute_block_positions()) {
                        assert(gameState.board.is_valid_spot(position));
                        auto boardIndex = position.y * gameState.board.columns + position.x;
                        gameState.board.data[boardIndex] = {gameState.currentShape.color, true};
                    }

                    auto const tspin = gameState.currentRotationType ? gameState.board.check_for_tspin(gameState.currentShape, *gameState.currentRotationType) : std::nullopt;

                    auto rowsCleared = gameState.board.remove_full_rows();
                    gameState.linesCleared += rowsCleared;
                    auto clearType = get_clear_type(rowsCleared, tspin);
                    auto constexpr clearTypeToName = std::array {
                        ""sv,
                        "Single"sv,
                        "Double"sv,
                        "Triple"sv,
                        "Tetris"sv,
                        "T-Spin"sv,
                        "T-Spin Single"sv,
                        "T-Spin Double"sv,
                        "T-Spin Triple"sv,
                        "T-Spin Mini"sv,
                        "T-Spin Mini Single"sv,
                        "T-Spin Mini Double"sv,
                    };
                    auto const clearName = clearTypeToName[static_cast<int>(clearType)];
                    if (!clearName.empty()) {
                        std::cout << clearName << std::endl;
                    }

                    // only regular clears count, but if it's a t-spin then
                    // droppedRows should have been set to 0 from rotating the shape
                    // so it SHOULDN'T be necessary to check explicitly.
                    if (clearType != ClearType::NONE) {
                        gameState.score += 2 * gameState.droppedRows;
                    }

                    // handle combos
                    switch (clearType) {
                        case ClearType::SINGLE:
                        case ClearType::DOUBLE:
                        case ClearType::TRIPLE:
                        case ClearType::TETRIS:
                        case ClearType::TSPIN_SINGLE:
                        case ClearType::TSPIN_DOUBLE:
                        case ClearType::TSPIN_TRIPLE:
                        case ClearType::TSPIN_MINI_SINGLE:
                        case ClearType::TSPIN_MINI_DOUBLE: {
                            ++gameState.comboCounter;
                            auto const comboScore = 50 * gameState.comboCounter * gameState.level;
                            gameState.score += comboScore;
                            if (comboScore) {
                                std::cerr << "Combo " << gameState.comboCounter << "! " << comboScore << " pts.\n";
                            }
                        } break;
                        default: {
                            gameState.comboCounter = -1;
                        } break;
                    }

                    // check for back to back tetris/t-spin
                    auto backToBackModifier = 1.0;
                    switch (clearType) {
                        case ClearType::TETRIS: {
                            if (gameState.backToBackType && (gameState.backToBackType == BackToBackType::TETRIS)) {
                                std::cerr << "Back to back Tetris\n";
                                backToBackModifier = 1.5;
                            } else {
                                gameState.backToBackType = BackToBackType::TETRIS;
                            }
                        } break;
                        case ClearType::TSPIN:
                        case ClearType::TSPIN_MINI:
                        case ClearType::TSPIN_SINGLE:
                        case ClearType::TSPIN_MINI_SINGLE:
                        case ClearType::TSPIN_DOUBLE:
                        case ClearType::TSPIN_MINI_DOUBLE:
                        case ClearType::TSPIN_TRIPLE: {
                            if (gameState.backToBackType && (gameState.backToBackType == BackToBackType::TSPIN)) {
                                std::cerr << "Back to back T-Spin\n";
                                backToBackModifier = 1.5;
                            } else {
                                gameState.backToBackType = BackToBackType::TSPIN;
                            }
                        } break;
                        default: {
                            gameState.backToBackType = {};
                        } break;
                    }

                    auto clearScore = calculate_score(clearType, gameState.level) * backToBackModifier;
                    gameState.score += clearScore;

                    gameState.level = gameState.linesCleared / 10 + 1;

                    gameState.currentShape = gameState.shapePool.next_shape();
                    // update shape shadow
                    gameState.currentShapeShadow = gameState.board.get_shadow(gameState.currentShape);
                    gameState.lockClock = frameStartClock;

                    gameState.hasHeld = false;

                    // game over if the new shape spawned on top of another shape
                    if (!gameState.board.is_valid_shape(gameState.currentShape)) {
                        gameOver = true;
                    }

                    if (gameOver) {
                        std::cout << "Game Over!\n";
                        if (gameState.score > highScore) highScore = gameState.score;
                        levelType = LevelType::MENU;
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

        switch (levelType) {
            case LevelType::MENU: {
                auto menuLabel = FontString::from_height_normalized("MENU", 1.f / 10.f);
                auto x = (1.f - menuLabel.normalizedW) / 2.f;
                auto y = 1.f / 10.f;
                draw_font_string_normalized(bb, menuLabel, x, y);
                if (!currentMenu) {
                    std::cerr << "ERROR: currentMenu is null, but levelType is LevelType::MENU\n";
                }
                for (auto& button : currentMenu->buttons) {
                    auto outlineScreenSpace = to_screen_space(button.dimensions);

                    draw_hollow_square(bb, outlineScreenSpace, {0, 0, 0});
                    draw_font_string(bb, button.text, outlineScreenSpace.x, outlineScreenSpace.y);
                }

                // draw high score
                using namespace std::string_literals;
                auto highScoreString = "High Score: "s + std::to_string(highScore);
                auto fontString = FontString::from_height_normalized(highScoreString, 0.048);
                draw_font_string_normalized(bb, fontString, 1.0 - fontString.normalizedW - 0.01, 0.01);
            } break;
            case LevelType::GAME: {
                // draw playarea background
                for (auto y = 2; y < Board::rows; ++y) {
                    for (auto x = 0; x < Board::columns; ++x) {
                        auto currindex = y * Board::columns + x;
                        auto& block = gameState.board.data[currindex];
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

                draw_shape_in_play_area(gameState.currentShapeShadow, 0xff / 2);
                draw_shape_in_play_area(gameState.currentShape, 0xff);

                // draw shape previews
                auto previewArray = gameState.shapePool.get_preview_shapes_array();
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
                auto scoreString = "Score: "s + std::to_string(gameState.score);
                auto scoreFontString = FontString::from_height_normalized(scoreString, 0.048);
                draw_font_string_normalized(bb, scoreFontString, 1.0 - scoreFontString.normalizedW - 0.01, 0.01);

                // draw level
                auto levelString = "Level: "s + std::to_string(gameState.level) + " (" + std::to_string(gameState.linesCleared) + "/" + std::to_string(gameState.level * 10) + ")";
                auto levelFontString = FontString::from_height_normalized(levelString, 0.048);
                draw_font_string_normalized(bb, levelFontString, 1.0 - levelFontString.normalizedW - 0.01, 0.01 + levelFontString.normalizedH);

                // draw held shape
                Squaref holdShapeDim{};
                holdShapeDim.x = gHoldShapeDim.x * scale;
                holdShapeDim.y = gHoldShapeDim.y * scale;
                holdShapeDim.w = gHoldShapeDim.w * scale;
                holdShapeDim.h = gHoldShapeDim.h * scale;
                draw_solid_square(bb, holdShapeDim, {0, 0, 0});
                if (gameState.holdShape) {
                    auto shape = *gameState.holdShape;
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
