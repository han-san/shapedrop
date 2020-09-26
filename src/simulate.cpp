#include <algorithm>
#include <iostream>
#include <string_view>
#include <cassert>

#include "fmt/core.h"

#include "core.hpp"
#include "ui.hpp"

#include "simulate.hpp"

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
    None,
    Single,
    Double,
    Triple,
    Tetris,

    Tspin,
    Tspin_single,
    Tspin_double,
    Tspin_triple,
    Tspin_mini,
    Tspin_mini_single,
    Tspin_mini_double,
};

std::array static constexpr clearTypeScores {
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

auto static get_clear_type(int const rowsCleared, std::optional<TspinType> const tspin) {
    assert(rowsCleared >= 0);
    assert(rowsCleared <= 4);
    if (!tspin) {
        return static_cast<ClearType>(rowsCleared);
    }

    assert(rowsCleared <= 3); // A t-spin can't clear more than 3 rows.
    switch (rowsCleared) {
        case 0:
            return *tspin == TspinType::Mini ? ClearType::Tspin_mini : ClearType::Tspin;
        case 1:
            return *tspin == TspinType::Mini ? ClearType::Tspin_mini_single : ClearType::Tspin_single;
        case 2:
            return *tspin == TspinType::Mini ? ClearType::Tspin_mini_double : ClearType::Tspin_double;
        case 3:
            // T-spin triple requires a wallkick so there is no
            // distinction between regular and mini (although it's
            // going to be represented internally as a mini).
            return ClearType::Tspin_triple;
        default: {
            // Getting here means rowsCleared was above 4 in a release build (BAD).
            auto const errMsg {fmt::format("The amount of rows cleared should be between 0 and 4, but is currently ({})", rowsCleared)};
            std::cerr << errMsg; // TODO: Log instead of stderr
            throw std::logic_error(errMsg);
        }
    }
}

auto static calculate_score(ClearType const clearType, int const level) {
    auto const index {static_cast<std::size_t>(clearType)};
    assert(index < clearTypeScores.size());
    return clearTypeScores[index] * level;
}

auto static calculate_score(int const rowsCleared, std::optional<TspinType> tspin, int const level) {
    auto const clearType {get_clear_type(rowsCleared, tspin)};
    return calculate_score(clearType, level);
}

auto simulate(ProgramState& programState, GameState& gameState, MenuState& menuState) -> void {
    if (programState.levelType == ProgramState::LevelType::Game) {
        auto const dropDelay = [&]() {
            auto const levelDropDelay {gameState.drop_delay_for_level()};
            if (gameState.isSoftDropping && (GameState::softDropDelay < levelDropDelay)) {
                return GameState::softDropDelay;
            } else {
                return levelDropDelay;
            }
        }();

        if (!gameState.paused) {
            // TODO: make it possible for shapes to drop more than one block
            // (e.g. at max drop speed it should drop all the way to the bottom
            // instantly)
            auto const nextdropClock {gameState.dropClock + dropDelay * CLOCKS_PER_SEC};
            if (programState.frameStartClock > nextdropClock) {
                gameState.dropClock = programState.frameStartClock;
                if (gameState.board.try_move(gameState.currentShape, {0, 1})) {
                    gameState.lockClock = programState.frameStartClock;
                    gameState.currentRotationType = std::nullopt;

                    if (gameState.isSoftDropping) {
                        ++gameState.softDropRowCount;
                    } else {
                        gameState.softDropRowCount = 0;
                    }
                }
            }

            if (programState.frameStartClock > gameState.lockClock + GameState::lockDelay) {
                // only care about locking if currentShape is on top of a block
                if (!gameState.board.is_valid_move(gameState.currentShape, {0, 1})) {
                    // game over if entire piece is above visible portion
                    // of board
                    auto const shapePositions {gameState.currentShape.get_absolute_block_positions()};
                    auto gameOver {
                        std::all_of(std::cbegin(shapePositions), std::cend(shapePositions), [](auto const& pos) {
                                    return pos.y < (Board::rows - Board::visibleRows);
                                    })
                    };

                    // fix currentBlocks position on board
                    for (auto const position : shapePositions) {
                        assert(gameState.board.is_valid_spot(position));
                        auto const boardIndex {static_cast<std::size_t>(position.y * gameState.board.columns + position.x)};
                        gameState.board.data[boardIndex] = {gameState.currentShape.color, true};
                    }

                    auto const tspin {gameState.currentRotationType ? gameState.board.check_for_tspin(gameState.currentShape, *gameState.currentRotationType) : std::nullopt};

                    auto const rowsCleared {gameState.board.remove_full_rows()};
                    gameState.linesCleared += static_cast<std::size_t>(rowsCleared);
                    auto const clearType {get_clear_type(rowsCleared, tspin)};
                    std::array static constexpr clearTypeToName {
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
                    auto const clearName {clearTypeToName[static_cast<std::size_t>(clearType)]};
                    if (!clearName.empty()) {
                        std::cout << clearName << std::endl;
                    }

                    // only regular clears count, but if it's a t-spin then
                    // droppedRows should have been set to 0 from rotating the shape
                    // so it SHOULDN'T be necessary to check explicitly.
                    if (clearType != ClearType::None) {
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
                        case ClearType::Single:
                        case ClearType::Double:
                        case ClearType::Triple:
                        case ClearType::Tetris:
                        case ClearType::Tspin_single:
                        case ClearType::Tspin_double:
                        case ClearType::Tspin_triple:
                        case ClearType::Tspin_mini_single:
                        case ClearType::Tspin_mini_double: {
                            ++gameState.comboCounter;
                            auto const comboScore {50 * gameState.comboCounter * gameState.level};
                            gameState.score += comboScore;
                            if (comboScore) {
                                fmt::print(stderr, "Combo {}! {} pts.\n", gameState.comboCounter, comboScore);
                            }
                        } break;
                            // These aren't technically clears and will reset your combo
                        case ClearType::None:
                        case ClearType::Tspin:
                        case ClearType::Tspin_mini: {
                            gameState.comboCounter = -1;
                        } break;
                    }

                    // check for back to back tetris/t-spin
                    auto backToBackModifier {1.0};
                    switch (clearType) {
                        case ClearType::Tetris: {
                            if (gameState.backToBackType && (gameState.backToBackType == BackToBackType::Tetris)) {
                                std::cerr << "Back to back Tetris\n";
                                backToBackModifier = 1.5;
                            } else {
                                gameState.backToBackType = BackToBackType::Tetris;
                            }
                        } break;
                        case ClearType::Tspin:
                        case ClearType::Tspin_mini:
                        case ClearType::Tspin_single:
                        case ClearType::Tspin_mini_single:
                        case ClearType::Tspin_double:
                        case ClearType::Tspin_mini_double:
                        case ClearType::Tspin_triple: {
                            if (gameState.backToBackType && (gameState.backToBackType == BackToBackType::Tspin)) {
                                std::cerr << "Back to back T-Spin\n";
                                backToBackModifier = 1.5;
                            } else {
                                gameState.backToBackType = BackToBackType::Tspin;
                            }
                        } break;
                        default: {
                            gameState.backToBackType = std::nullopt;
                        } break;
                    }

                    auto clearScore {static_cast<int>(calculate_score(clearType, gameState.level) * backToBackModifier)};
                    gameState.score += clearScore;

                    gameState.level = gameState.linesCleared / 10 + gameState.startingLevel;

                    gameState.currentShape = gameState.shapePool.next_shape();
                    // update shape shadow
                    gameState.currentShapeShadow = gameState.board.get_shadow(gameState.currentShape);

                    gameState.lockClock = programState.frameStartClock;

                    gameState.hasHeld = false;

                    // game over if the new shape spawned on top of another shape
                    if (!gameState.board.is_valid_shape(gameState.currentShape)) {
                        gameOver = true;
                    }

                    if (gameOver) {
                        std::cout << "Game Over!\n";
                        if (gameState.score > programState.highScore) {
                            programState.highScore = gameState.score;
                        }
                        programState.levelType = ProgramState::LevelType::Menu;
                    }

                }
            }

        }

        {
            auto const fontSize {0.048};
            UI::label(fmt::format("Score: {}", gameState.score), fontSize, UI::XAlignment::Right);

            // Round up linesCleared to nearest 10
            auto const linesRequired {(gameState.linesCleared / 10 + 1) * 10};
            // Not const since it's later moved
            auto levelString {fmt::format("Level: {} ({}/{})", gameState.level, gameState.linesCleared, linesRequired)};
            UI::label(std::move(levelString), fontSize, UI::XAlignment::Right, fontSize);
        }

        if (gameState.paused) {
            UI::begin_menu({0.2, 0.2, 0.6, 0.6}, Color::cyan);
            UI::label("Paused", 0.06, UI::XAlignment::Center);
            if (UI::button("Resume", 0.06, UI::XAlignment::Center)) {
                gameState.paused = false;

                // TODO: maybe save the amount of clocks left when the game was paused and set them again here.
                gameState.dropClock = programState.frameStartClock;
                gameState.lockClock = programState.frameStartClock;
            }
            if (UI::button("Main Menu", 0.06, UI::XAlignment::Center)) {
                programState.levelType = ProgramState::LevelType::Menu;
            }
            if (UI::button("Quit", 0.06, UI::XAlignment::Center)) {
                programState.running = false;
            }

            UI::end_menu();
        }

    } else if (programState.levelType == ProgramState::LevelType::Menu) {
        auto const highScoreFontSize {0.048};
        UI::label(fmt::format("High Score: {}", programState.highScore), highScoreFontSize, UI::XAlignment::Right);

        auto const menuY {1. / 10.};
        auto const menuFontSize {1. / 10.};
        UI::begin_menu({0., menuY, 1., 1. - menuY});
        UI::label("ShapeDrop", menuFontSize, UI::XAlignment::Center);
        if (UI::button("Play", menuFontSize, UI::XAlignment::Center)) {
            // FIXME: This will cause the game field to render this frame,
            // but the UI being rendered this frame will be the main menu's
            // instead of the game field's since that's the simulation
            // branch we're currently on.
            programState.levelType = ProgramState::LevelType::Game;
            gameState = GameState{menuState.level};
        }
        UI::spinbox("Level", menuFontSize / 2., UI::XAlignment::Center, 0., menuState.level, gMinLevel, gMaxLevel);
        UI::end_menu();
    }

}
