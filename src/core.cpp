#include <array>
#include <cassert>
#include <ctime>
#include <iostream>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "fmt/core.h"

#include "jint.h"

#include "board.hpp"
#include "draw.hpp"
#include "platform.hpp"
#include "shape.hpp"
#include "tests.hpp"
#include "ui.hpp"
#include "util.hpp"

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
//
// Softdropping.
// dropcount needs to be reset when:
//     A new shape is introduced, i.e. shape lock or hold.
//     the soft drop button is released, but not if the shape is on ground.
//     If the shape falls at all when the soft drop button is not pressed.
//     If the shape is moved when it's grounded or is rotated with a kick while grounded
//     If the shape is hard dropped (and actually moves from it)

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

auto get_clear_type(uint const rowsCleared, std::optional<TspinType> const tspin) {
    assert(rowsCleared <= 4);
    if (!tspin) {
        return static_cast<ClearType>(rowsCleared);
    }

    assert(rowsCleared <= 3); // A t-spin can't clear more than 3 rows.
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
        default: {
            // Getting here means rowsCleared was above 4 in a release build (BAD).
            auto const errMsg = fmt::format("The amount of rows cleared should be between 0 and 4, but is currently ({})", rowsCleared);
            std::cerr << errMsg; // TODO: Log instead of stderr
            throw std::logic_error(errMsg);
        } break;
    }
}

auto calculate_score(ClearType const clearType, int const level) {
    auto const index = static_cast<std::size_t>(clearType);
    assert(index < clearTypeScores.size());
    return clearTypeScores[index] * level;
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
auto constexpr static initialDropDelay = 1.0;
auto constexpr static softDropDelay = 0.1;
bool running = true;

std::array<Shape, ShapePool::SIZE> const initialShapes {
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

auto constexpr gMinLevel = 1;
auto constexpr gMaxLevel = 99;

struct MenuState {
    size_t level = gMinLevel;
};
auto menuState = MenuState{};

// For variables which are unique to their instance of a game
// i.e. should be reset when starting a new one
struct GameState {

    // unique to current shape
    time_t dropClock = clock();
    time_t lockClock = dropClock;
    size_t droppedRows = 0;
    size_t softDropRowCount = 0;

    // shared for all shapes
    bool isSoftDropping = false;
    size_t linesCleared = 0;
    size_t startingLevel = menuState.level;
    size_t level = startingLevel;
    size_t score = 0;
    bool hasHeld = false;
    std::optional<BackToBackType> backToBackType = {};
    // Starts at -1 since the first clear advances the counter, but only the
    // second clear in a row counts as a combo.
    int comboCounter = -1;
    Board board = {};
    ShapePool shapePool = {initialShapes};
    Shape currentShape = shapePool.current_shape();
    Shape currentShapeShadow = board.get_shadow(currentShape);
    std::optional<Shape::RotationType> currentRotationType = {};
    std::optional<Shape> holdShape = {};
    bool paused = false;

    auto reset() {
        *this = GameState {};
    }

    auto drop_delay_for_level() const {
        auto const dropDelay = initialDropDelay - this->level * 0.1;
        // dropDelay can't be negative
        return dropDelay > 0. ? dropDelay : 0.;
    }
};

auto run() -> void
{
    tests::run();

    GameState gameState = {};

    time_t frameStartClock = clock();

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
            UI::update_state(message);

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

                enum class HorDir {
                    LEFT,
                    RIGHT
                };

                auto move_horizontal = [&] (HorDir const dir) {
                    // if currentShape is on top of a block before move,
                    // the drop clock needs to be reset
                    auto isGrounded = !gameState.board.is_valid_move(gameState.currentShape, {0, 1});
                    auto const dirVec = dir == HorDir::RIGHT ? V2{1, 0} : V2{-1, 0};
                    if (gameState.board.try_move(gameState.currentShape, dirVec)) {
                        update_shadow_and_clocks(isGrounded);
                        // if you move the piece you cancel the drop
                        gameState.droppedRows = 0;
                        if (isGrounded) {
                            gameState.softDropRowCount = 0;
                        }
                    }
                };

                auto rotate_current_shape = [&] (Shape::Rotation rot) {
                    // if currentShape is on top of a block before rotation,
                    // the drop clock needs to be reset
                    auto isGrounded = !gameState.board.is_valid_move(gameState.currentShape, {0, 1});
                    if (auto const rotation = gameState.board.rotate_shape(gameState.currentShape, rot); rotation) {
                        update_shadow_and_clocks(isGrounded);
                        gameState.currentRotationType = rotation;
                        // if you rotate the piece you cancel the drop
                        gameState.droppedRows = 0;
                        if ((rotation == Shape::RotationType::WALLKICK) && isGrounded) {
                            gameState.softDropRowCount = 0;
                        }
                    }
                };

                if (message.type == Message::Type::MOVE_RIGHT) {
                    move_horizontal(HorDir::RIGHT);
                } else if (message.type == Message::Type::MOVE_LEFT) {
                    move_horizontal(HorDir::LEFT);
                } else if (message.type == Message::Type::INCREASE_SPEED) {
                    // TODO: How does this work if you e.g. press
                    // left/right/rotate while holding button down?
                    // is isSoftDropping still true at that time?

                    // This message currently gets spammed when you hold down
                    // the button, so resetting the soft drop count directly
                    // will continue resetting it while the button is pressed.
                    // In order to avoid that we check if isSoftDropping has
                    // been set, which only happens during spam.
                    if (!gameState.isSoftDropping) {
                        gameState.softDropRowCount = 0;
                    }
                    gameState.isSoftDropping = true;
                } else if (message.type == Message::Type::RESET_SPEED) {
                    gameState.isSoftDropping = false;

                    // softdrops only get reset if the piece can currently fall
                    if (gameState.board.is_valid_move(gameState.currentShape, {0, 1})) {
                        gameState.softDropRowCount = 0;
                    }
                } else if (message.type == Message::Type::DROP) {
                    auto droppedRows = 0;
                    while (gameState.board.try_move(gameState.currentShape, {0, 1})) {
                        gameState.lockClock = frameStartClock;
                        gameState.currentRotationType = {};
                        ++droppedRows;
                    }
                    gameState.droppedRows = droppedRows;

                    // hard drop overrides soft drop
                    if (droppedRows) {
                        gameState.softDropRowCount = 0;
                    }
                } else if (message.type == Message::Type::ROTATE_LEFT) {
                    rotate_current_shape(Shape::Rotation::LEFT);
                } else if (message.type == Message::Type::ROTATE_RIGHT) {
                    rotate_current_shape(Shape::Rotation::RIGHT);
                } else if (message.type == Message::Type::ROTATE_RIGHT) {
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

                        gameState.softDropRowCount = 0;
                        gameState.droppedRows = 0;

                        auto isGrounded = !gameState.board.is_valid_move(gameState.currentShape, {0, 1});
                        update_shadow_and_clocks(isGrounded);
                    }
                } else if (message.type == Message::Type::PAUSE) {
                    gameState.paused = !gameState.paused;
                    // TODO: maybe save the amount of clocks left when the game was paused and set them again here.
                    gameState.dropClock = frameStartClock;
                    gameState.lockClock = frameStartClock;
                }
            }
        }

        // sim
        if (levelType == LevelType::GAME) {
            auto const dropDelay = [&]() {
                auto const levelDropDelay = gameState.drop_delay_for_level();
                if (gameState.isSoftDropping && (softDropDelay < levelDropDelay)) {
                    return softDropDelay;
                } else {
                    return levelDropDelay;
                }
            }();

            if (!gameState.paused) {
                // TODO: make it possible for shapes to drop more than one block
                // (e.g. at max drop speed it should drop all the way to the bottom
                // instantly)
                auto nextdropClock = gameState.dropClock + dropDelay * CLOCKS_PER_SEC;
                if (frameStartClock > nextdropClock) {
                    gameState.dropClock = frameStartClock;
                    if (gameState.board.try_move(gameState.currentShape, {0, 1})) {
                        gameState.lockClock = frameStartClock;
                        gameState.currentRotationType = {};

                        if (gameState.isSoftDropping) {
                            ++gameState.softDropRowCount;
                        } else {
                            gameState.softDropRowCount = 0;
                        }
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
                            // you shouldn't be able to soft drop and hard drop at the same time.
                            assert(!gameState.droppedRows || !gameState.softDropRowCount);
                            gameState.score += 2 * gameState.droppedRows;
                            gameState.score += gameState.softDropRowCount;
                        }
                        // needs to be reset for the next piece
                        gameState.softDropRowCount = 0;
                        gameState.droppedRows = 0;

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
                                    fmt::print(stderr, "Combo {}! {} pts.\n", gameState.comboCounter, comboScore);
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

                        auto clearScore = size_t(calculate_score(clearType, gameState.level) * backToBackModifier);
                        gameState.score += clearScore;

                        gameState.level = gameState.linesCleared / 10 + gameState.startingLevel;

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

            {
                auto const fontSize = 0.048f;
                UI::label(fmt::format("Score: {}", gameState.score), fontSize, UI::XAlignment::RIGHT);

                // Round up linesCleared to nearest 10
                auto const linesRequired = (gameState.linesCleared / 10 + 1) * 10;
                auto levelString = fmt::format("Level: {} ({}/{})", gameState.level, gameState.linesCleared, linesRequired);
                UI::label(std::move(levelString), fontSize, UI::XAlignment::RIGHT, fontSize);
            }

            if (gameState.paused) {
                UI::begin_menu({0.2f, 0.2f, 0.6f, 0.6f}, {0x00, 0xff, 0xff, 0xff});
                UI::label("Paused", 0.06f, UI::XAlignment::CENTER);
                if (UI::button("Resume", 0.06f, UI::XAlignment::CENTER)) {
                    gameState.paused = false;

                    // TODO: maybe save the amount of clocks left when the game was paused and set them again here.
                    gameState.dropClock = frameStartClock;
                    gameState.lockClock = frameStartClock;
                }
                if (UI::button("Main Menu", 0.06f, UI::XAlignment::CENTER)) {
                    levelType = LevelType::MENU;
                }
                if (UI::button("Quit", 0.06f, UI::XAlignment::CENTER)) {
                    running = false;
                }

                UI::end_menu();
            }

        } else if (levelType == LevelType::MENU) {
            auto const highScoreFontSize = 0.048f;
            UI::label(fmt::format("High Score: {}", highScore), highScoreFontSize, UI::XAlignment::RIGHT);

            auto menuY = 1.f / 10.f;
            auto menuFontSize = 1.f / 10.f;
            UI::begin_menu({0.f, menuY, 1.f, 1.f - menuY});
            UI::label("ShapeDrop", menuFontSize, UI::XAlignment::CENTER);
            if (UI::button("Play", menuFontSize, UI::XAlignment::CENTER)) {
                // FIXME: This will cause the game field to render this frame,
                // but the UI being rendered this frame will be the main menu's
                // instead of the game field's since that's the simulation
                // branch we're currently on.
                levelType = LevelType::GAME;
                gameState.reset();
            }
            UI::spinbox("Level", menuFontSize / 2.f, UI::XAlignment::CENTER, 0.f, menuState.level, gMinLevel, gMaxLevel);
            UI::end_menu();
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

                // draw held shape
                Squaref holdShapeDim{};
                holdShapeDim.x = gHoldShapeDim.x * float(scale);
                holdShapeDim.y = gHoldShapeDim.y * float(scale);
                holdShapeDim.w = gHoldShapeDim.w * float(scale);
                holdShapeDim.h = gHoldShapeDim.h * float(scale);
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

        UI::draw(bb);

        swap_buffer();
    }
}
