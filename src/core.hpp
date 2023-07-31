#pragma once

#include "board.hpp"
#include "shape.hpp"
#include "util.hpp"

#include <array>
#include <chrono>
#include <optional>

struct BackBuffer {
  void* memory {};
  Rect<PositiveUInt>::Size dimensions;
  PositiveUInt pitch {};
  PositiveU8 bpp {};
};

constexpr auto gBorderSize = 1;

constexpr Rect<int> gHoldShapeDim {gBorderSize, gBorderSize, 5, 3};
constexpr Rect<int> gPlayAreaDim {gBorderSize, (gBorderSize + gHoldShapeDim.h + gBorderSize),
                                  Board::columns, Board::visibleRows};
constexpr Rect<int> gSidebarDim {(gBorderSize + gPlayAreaDim.w + gBorderSize), gBorderSize, 4,
                                 (gHoldShapeDim.h + gBorderSize + gPlayAreaDim.h)};

constexpr auto gBaseWindowWidth =
    gBorderSize + gPlayAreaDim.w + gBorderSize + gSidebarDim.w + gBorderSize;
constexpr auto gBaseWindowHeight =
    gBorderSize + gHoldShapeDim.h + gBorderSize + gPlayAreaDim.h + gBorderSize;

static constexpr ShapePool::DataType initialShapes {
    Shape::Type::I, Shape::Type::L, Shape::Type::J, Shape::Type::O,
    Shape::Type::S, Shape::Type::Z, Shape::Type::T,
};

enum class BackToBackType { Tetris, Tspin };

constexpr auto gMinLevel = 1;
constexpr auto gMaxLevel = 99;

struct MenuState {
  int level {gMinLevel};
};

struct ProgramState {
  enum class LevelType {
    Menu,
    Game,
  };

  using HiResClock = std::chrono::high_resolution_clock;
  HiResClock::time_point frameStartClock {HiResClock::now()};
  HiResClock::duration frameTime {0};
  static constexpr uint targetFPS {60};
  static constexpr HiResClock::duration targetFrameTime {
      std::chrono::duration_cast<HiResClock::duration>(
          std::chrono::duration<double> {1. / targetFPS})};

  LevelType levelType {LevelType::Menu};
  bool running {true};
  int highScore {0};
};

// For variables which are unique to their instance of a game
// i.e. should be reset when starting a new one
struct GameState {

  explicit GameState(int sstartingLevel) : startingLevel {sstartingLevel} {}

  // unique to current shape
  using HiResClock = std::chrono::high_resolution_clock;
  HiResClock::time_point dropClock {HiResClock::now()};
  HiResClock::time_point lockClock {dropClock};
  int droppedRows {0};
  int softDropRowCount {0};

  // shared for all shapes
  static constexpr std::chrono::milliseconds lockDelay {500};
  static constexpr std::chrono::milliseconds softDropDelay {100};
  static constexpr std::chrono::seconds initialDropDelay {1};

  bool isSoftDropping {false};
  int linesCleared {0};
  int startingLevel {1};
  int level {startingLevel};
  int score {0};
  bool hasHeld {false};
  std::optional<BackToBackType> backToBackType {};
  // Starts at -1 since the first clear advances the counter, but only the
  // second clear in a row counts as a combo.
  int comboCounter {-1};
  Board board {};
  ShapePool shapePool {initialShapes};
  Shape currentShape {shapePool.current_shape()};
  Shape currentShapeShadow {board.get_shadow(currentShape)};
  std::optional<Shape::RotationType> currentRotationType {};
  std::optional<Shape::Type> holdShapeType {};
  bool paused {false};

  auto reset() { *this = GameState {startingLevel}; }

  [[nodiscard]] auto drop_delay_for_level() const {
    using namespace std::chrono_literals;
    const auto dropDelay = initialDropDelay - (this->level * 100ms);
    // dropDelay can't be negative
    return dropDelay > 0s ? dropDelay : 0s;
  }
};

auto run() -> void;
