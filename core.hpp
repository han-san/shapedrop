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
        NONE,
        QUIT,
        RESET,
        MOVE_RIGHT,
        MOVE_LEFT,
        INCREASE_SPEED,
        RESET_SPEED,
        DROP,
        ROTATE_LEFT,
        ROTATE_RIGHT,
        INCREASE_WINDOW_SIZE,
        DECREASE_WINDOW_SIZE,
        MOUSEBUTTONDOWN,
    };

    Type type;
    int x;
    int y;
};

auto constexpr baseWindowWidth = gColumns + 2 + 5; // space for border and sidebar
auto constexpr baseWindowHeight = gRows + 2;

auto run() -> void;
