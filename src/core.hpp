#pragma once

#include <array>
#include <chrono>
#include <optional>

#include "board.hpp"
#include "util.hpp"
#include "shape.hpp"

struct BackBuffer {
    void* memory {};
    PositiveUInt w {};
    PositiveUInt h {};
    PositiveUInt pitch {};
    PositiveU8 bpp {};
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

std::array<Shape, ShapePool::size> static const initialShapes {
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

    using HiResClock = std::chrono::high_resolution_clock;
    HiResClock::time_point frameStartClock {HiResClock::now()};
    HiResClock::duration frameTime {0};
    uint static constexpr targetFPS {60};
    HiResClock::duration static constexpr targetFrameTime {
        std::chrono::duration_cast<HiResClock::duration>(std::chrono::duration<double> {1. / targetFPS})
    };

    LevelType levelType {LevelType::Menu};
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
    using HiResClock = std::chrono::high_resolution_clock;
    HiResClock::time_point dropClock {HiResClock::now()};
    HiResClock::time_point lockClock {dropClock};
    int droppedRows {0};
    int softDropRowCount {0};

    // shared for all shapes
    std::chrono::milliseconds static constexpr lockDelay {500};
    std::chrono::milliseconds static constexpr softDropDelay {100};
    std::chrono::seconds static constexpr initialDropDelay {1};

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

    auto reset() {
        *this = GameState {startingLevel};
    }

    [[nodiscard]] auto drop_delay_for_level() const {
        using namespace std::chrono_literals;
        auto const dropDelay {initialDropDelay - (this->level * 100ms)};
        // dropDelay can't be negative
        return dropDelay > 0s ? dropDelay : 0s;
    }
};

auto run() -> void;
