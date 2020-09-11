#pragma once

#include "board.hpp"

struct BackBuffer {
    void* memory;
    int w;
    int h;
    int pitch;
    int bpp;
};

struct Message {
    enum class Type {
        None,
        Quit,
        Reset,
        Hold,
        Move_right,
        Move_left,
        Increase_speed,
        Reset_speed,
        Drop,
        Rotate_left,
        Rotate_right,
        Increase_window_size,
        Decrease_window_size,
        Mousebuttondown,
        Pause,
    };

    Type type;
    int x;
    int y;
};

auto constexpr gBorderSize {1};

Square constexpr gHoldShapeDim {gBorderSize, gBorderSize, 5, 3};
Square constexpr gPlayAreaDim {
    gBorderSize, (gBorderSize + gHoldShapeDim.h + gBorderSize),
    Board::columns, Board::visibleRows
};
Square constexpr gSidebarDim {
    (gBorderSize + gPlayAreaDim.w + gBorderSize), gBorderSize,
    4, (gHoldShapeDim.h + gBorderSize + gPlayAreaDim.h)
};

auto inline constexpr constexpr_round(double const val) -> int {
    return int(val + 0.5);
}

auto constexpr gBaseWindowWidth {constexpr_round(double(gBorderSize + gPlayAreaDim.w + gBorderSize + gSidebarDim.w + gBorderSize))};
auto constexpr gBaseWindowHeight {constexpr_round(double(gBorderSize + gHoldShapeDim.h + gBorderSize + gPlayAreaDim.h + gBorderSize))};

auto run() -> void;
