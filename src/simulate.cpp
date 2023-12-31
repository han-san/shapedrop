#include "simulate.hpp"

#include "core.hpp"
#include "rangealgorithms.hpp"
#include "ui.hpp"

#include "fmt/core.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>

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
// Back to Back Tetris/T-Spin: * 1.5 (e.g. back to back tetris: 1200 x level)
// Combo:     50 x combo count x level
// Soft drop: 1 point per cell
// Hard drop: 2 point per cell
//
// Softdropping.
// dropcount needs to be reset when:
//     A new shape is introduced, i.e. shape lock or hold.
//     the soft drop button is released, but not if the shape is on ground.
//     If the shape falls at all when the soft drop button is not pressed.
//     If the shape is moved when it's grounded or is rotated with a kick while
//     grounded If the shape is hard dropped (and actually moves from it)

enum class ClearType {
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

auto static clear_type_to_score(ClearType const c) -> int {
  auto constexpr None = 0;
  auto constexpr Single = 100;
  auto constexpr Double = 300;
  auto constexpr Triple = 500;
  auto constexpr Tetris = 800;
  auto constexpr Tspin = 400;
  auto constexpr Tspin_single = 800;
  auto constexpr Tspin_double = 1200;
  auto constexpr Tspin_triple = 1600;
  auto constexpr Tspin_mini = 100;
  auto constexpr Tspin_mini_single = 200;
  auto constexpr Tspin_mini_double = 1200;

  switch (c) {
  case ClearType::None:
    return None;
  case ClearType::Single:
    return Single;
  case ClearType::Double:
    return Double;
  case ClearType::Triple:
    return Triple;
  case ClearType::Tetris:
    return Tetris;
  case ClearType::Tspin:
    return Tspin;
  case ClearType::Tspin_single:
    return Tspin_single;
  case ClearType::Tspin_double:
    return Tspin_double;
  case ClearType::Tspin_triple:
    return Tspin_triple;
  case ClearType::Tspin_mini:
    return Tspin_mini;
  case ClearType::Tspin_mini_single:
    return Tspin_mini_single;
  case ClearType::Tspin_mini_double:
    return Tspin_mini_double;
  }
  // Unreachable.
  std::terminate();
}

auto static to_string_view(ClearType const c) -> std::string_view {
  switch (c) {
  case ClearType::None:
    return "";
  case ClearType::Single:
    return "Single";
  case ClearType::Double:
    return "Double";
  case ClearType::Triple:
    return "Triple";
  case ClearType::Tetris:
    return "Tetris";
  case ClearType::Tspin:
    return "Tspin";
  case ClearType::Tspin_single:
    return "Tspin_single";
  case ClearType::Tspin_double:
    return "Tspin_double";
  case ClearType::Tspin_triple:
    return "Tspin_triple";
  case ClearType::Tspin_mini:
    return "Tspin_mini";
  case ClearType::Tspin_mini_single:
    return "Tspin_mini_single";
  case ClearType::Tspin_mini_double:
    return "Tspin_mini_double";
  }
  // Unreachable.
  std::terminate();
}

[[nodiscard]] auto static get_clear_type(int const rowsCleared,
                                         std::optional<TspinType> const tspin) {
  auto bad_row_count_msg = [rowsCleared](std::size_t min, std::size_t max) {
    return fmt::format("The amount of rows cleared should be between {} and "
                       "{}, but is currently {}",
                       min, max, rowsCleared);
  };

  if (not tspin) {
    switch (rowsCleared) {
    case 0:
      return ClearType::None;
    case 1:
      return ClearType::Single;
    case 2:
      return ClearType::Double;
    case 3:
      return ClearType::Triple;
    case 4:
      return ClearType::Tetris;
    default:
      auto const errMsg = bad_row_count_msg(0, 4);
      throw std::invalid_argument(errMsg);
    }
  }

  if (*tspin == TspinType::Mini) {
    switch (rowsCleared) {
    case 0:
      return ClearType::Tspin_mini;
    case 1:
      return ClearType::Tspin_mini_single;
    case 2:
      return ClearType::Tspin_mini_double;
    case 3:
      // T-spin triple requires a wallkick so there is no
      // distinction between regular and mini (although it's
      // going to be represented internally as a mini).
      return ClearType::Tspin_triple;
    default:
      auto errMsg = bad_row_count_msg(0, 3);
      throw std::invalid_argument(errMsg);
    }
  }

  switch (rowsCleared) {
  case 0:
    return ClearType::Tspin;
  case 1:
    return ClearType::Tspin_single;
  case 2:
    return ClearType::Tspin_double;
  case 3:
    return ClearType::Tspin_triple;
  default:
    auto errMsg = bad_row_count_msg(0, 3);
    throw std::invalid_argument(errMsg);
  }
}

[[nodiscard]] auto static calculate_score(ClearType const clearType,
                                          int const level) {
  return clear_type_to_score(clearType) * level;
}

auto static lock_current_shape(GameState& gameState, ProgramState& programState)
    -> void {
  // game over if entire piece is above visible portion
  // of board
  auto const shapePositions =
      gameState.currentShape.get_absolute_block_positions();
  auto gameOver = all_of(shapePositions, [](auto const& pos) {
    return pos.y < (Board::rows - Board::visibleRows);
  });

  // fix currentBlocks position on board
  for (auto const position : shapePositions) {
    assert(gameState.board.is_valid_spot(position));
    gsl::index index {position.y * gameState.board.columns + position.x};
    gameState.board.block_at(index) = {gameState.currentShape.color, true};
  }

  auto const tspin =
      gameState.currentRotationType
          ? gameState.board.check_for_tspin(gameState.currentShape,
                                            *gameState.currentRotationType)
          : std::nullopt;

  auto const rowsCleared = gameState.board.remove_full_rows();
  gameState.linesCleared += rowsCleared;
  auto const clearType = get_clear_type(rowsCleared, tspin);
  auto const clearName = to_string_view(clearType);
  if (not clearName.empty()) {
    std::cout << clearName << std::endl;
  }

  // only regular clears count, but if it's a t-spin then
  // droppedRows should have been set to 0 from rotating the shape
  // so it SHOULDN'T be necessary to check explicitly.
  if (clearType != ClearType::None) {
    // you shouldn't be able to soft drop and hard drop at the same
    // time.
    assert(not gameState.droppedRows or not gameState.softDropRowCount);
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
    auto const comboScore = 50 * gameState.comboCounter * gameState.level;
    gameState.score += comboScore;
    if (comboScore) {
      fmt::print(stderr, "Combo {}! {} pts.\n", gameState.comboCounter,
                 comboScore);
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
  auto backToBackModifier = 1.0;
  switch (clearType) {
  case ClearType::Tetris: {
    if (gameState.backToBackType == BackToBackType::Tetris) {
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
    if (gameState.backToBackType == BackToBackType::Tspin) {
      std::cerr << "Back to back T-Spin\n";
      backToBackModifier = 1.5;
    } else {
      gameState.backToBackType = BackToBackType::Tspin;
    }
  } break;
  case ClearType::None:
  case ClearType::Single:
  case ClearType::Double:
  case ClearType::Triple: {
    gameState.backToBackType = std::nullopt;
  } break;
  }

  auto clearScore = static_cast<int>(
      calculate_score(clearType, gameState.level) * backToBackModifier);
  gameState.score += clearScore;

  gameState.level = gameState.linesCleared / 10 + gameState.startingLevel;

  gameState.currentShape = gameState.shapePool.next_shape();
  // update shape shadow
  gameState.currentShapeShadow =
      gameState.board.get_shadow(gameState.currentShape);

  gameState.lockClock = programState.frameStartClock;

  gameState.hasHeld = false;

  // game over if the new shape spawned on top of another shape
  if (not gameState.board.is_valid_shape(gameState.currentShape)) {
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

auto static simulate_game(ProgramState& programState, GameState& gameState)
    -> void {
  auto const dropDelay = [&]() {
    auto const levelDropDelay = gameState.drop_delay_for_level();
    if (gameState.isSoftDropping and
        (GameState::softDropDelay < levelDropDelay)) {
      return GameState::softDropDelay;
    } else {
      return levelDropDelay;
    }
  }();

  if (not gameState.paused) {
    // TODO: make it possible for shapes to drop more than one block
    // (e.g. at max drop speed it should drop all the way to the bottom
    // instantly)
    auto const nextdropClock = gameState.dropClock + dropDelay;
    if (programState.frameStartClock > nextdropClock) {
      gameState.dropClock = programState.frameStartClock;
      if (gameState.board.try_move(gameState.currentShape, V2::down())) {
        gameState.lockClock = programState.frameStartClock;
        gameState.currentRotationType = std::nullopt;

        if (gameState.isSoftDropping) {
          ++gameState.softDropRowCount;
        } else {
          gameState.softDropRowCount = 0;
        }
      }
    }

    if (programState.frameStartClock >
        gameState.lockClock + GameState::lockDelay) {
      // only care about locking if currentShape is on top of a block
      if (not gameState.board.is_valid_move(gameState.currentShape,
                                            V2::down())) {
        lock_current_shape(gameState, programState);
      }
    }
  }

  {
    auto const fontSize = 0.048;
    UI::label(fmt::format("Score: {}", gameState.score), fontSize,
              UI::XAlignment::Right);

    // Round up linesCleared to nearest 10
    auto const linesRequired = (gameState.linesCleared / 10 + 1) * 10;
    // Not const since it's later moved
    auto levelString = fmt::format("Level: {} ({}/{})", gameState.level,
                                   gameState.linesCleared, linesRequired);
    UI::label(std::move(levelString), fontSize, UI::XAlignment::Right,
              fontSize);
  }

  if (gameState.paused) {
    UI::begin_menu({0.2, 0.2, 0.6, 0.6}, Color::cyan);
    UI::label("Paused", 0.06, UI::XAlignment::Center);
    if (UI::button("Resume", 0.06, UI::XAlignment::Center)) {
      gameState.paused = false;

      // TODO: maybe save the amount of clocks left when the game was paused
      // and set them again here.
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
}

auto static simulate_menu(ProgramState& programState, GameState& gameState,
                          MenuState& menuState) -> void {
  auto const highScoreFontSize = 0.048;
  UI::label(fmt::format("High Score: {}", programState.highScore),
            highScoreFontSize, UI::XAlignment::Right);

  auto const menuY = 1. / 10.;
  auto const menuFontSize = 1. / 10.;
  UI::begin_menu({0., menuY, 1., 1. - menuY});
  UI::label("ShapeDrop", menuFontSize, UI::XAlignment::Center);
  if (UI::button("Play", menuFontSize, UI::XAlignment::Center)) {
    // FIXME: This will cause the game field to render this frame,
    // but the UI being rendered this frame will be the main menu's
    // instead of the game field's since that's the simulation
    // branch we're currently on.
    programState.levelType = ProgramState::LevelType::Game;
    gameState = GameState {menuState.level};
  }
  UI::spinbox("Level", menuFontSize / 2., UI::XAlignment::Center, 0.,
              menuState.level, gMinLevel, gMaxLevel);
  UI::end_menu();
}

auto simulate(ProgramState& programState, GameState& gameState,
              MenuState& menuState) -> void {
  switch (programState.levelType) {
  case ProgramState::LevelType::Game:
    simulate_game(programState, gameState);
    break;
  case ProgramState::LevelType::Menu:
    simulate_menu(programState, gameState, menuState);
    break;
  }
}
