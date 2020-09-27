#pragma once

#include <ctime>

#include "board.hpp"

struct BackBuffer {
    void* memory;
    int w;
    int h;
    int pitch;
    int bpp;
};

auto constexpr gBorderSize {1};

Rect<int> constexpr gHoldShapeDim {gBorderSize, gBorderSize, 5, 3};
Rect<int> constexpr gPlayAreaDim {
    gBorderSize, (gBorderSize + gHoldShapeDim.h + gBorderSize),
    Board::columns, Board::visibleRows
};
Rect<int> constexpr gSidebarDim {
    (gBorderSize + gPlayAreaDim.w + gBorderSize), gBorderSize,
    4, (gHoldShapeDim.h + gBorderSize + gPlayAreaDim.h)
};

auto constexpr gBaseWindowWidth {gBorderSize + gPlayAreaDim.w + gBorderSize + gSidebarDim.w + gBorderSize};
auto constexpr gBaseWindowHeight {gBorderSize + gHoldShapeDim.h + gBorderSize + gPlayAreaDim.h + gBorderSize};

std::array<Shape, ShapePool::SIZE> static const initialShapes {
    Shape::Type::I,
    Shape::Type::L,
    Shape::Type::J,
    Shape::Type::O,
    Shape::Type::S,
    Shape::Type::Z,
    Shape::Type::T,
};

enum class BackToBackType {
    Tetris,
    Tspin
};

auto constexpr gMinLevel {1};
auto constexpr gMaxLevel {99};

struct MenuState {
    int level {gMinLevel};
};

struct ProgramState {
    enum class LevelType {
        Menu,
        Game,
    };

    LevelType levelType {LevelType::Menu};
    clock_t frameStartClock = clock();
    bool running {true};
    int highScore {0};
};

// For variables which are unique to their instance of a game
// i.e. should be reset when starting a new one
struct GameState {

    explicit GameState(int sstartingLevel)
    : startingLevel{sstartingLevel}
    {}

    // unique to current shape
    time_t dropClock {clock()};
    time_t lockClock {dropClock};
    int droppedRows {0};
    int softDropRowCount {0};

    // shared for all shapes
    time_t static constexpr lockDelay {CLOCKS_PER_SEC / 2};
    double static constexpr softDropDelay {0.1};
    double static constexpr initialDropDelay {1.0};

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
    std::optional<Shape> holdShape {};
    bool paused {false};

    auto reset() {
        *this = GameState {startingLevel};
    }

    [[nodiscard]] auto drop_delay_for_level() const {
        auto const dropDelay {initialDropDelay - this->level * 0.1};
        // dropDelay can't be negative
        return dropDelay > 0. ? dropDelay : 0.;
    }
};

auto run() -> void;
