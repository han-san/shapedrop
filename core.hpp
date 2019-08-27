#pragma once

struct BackBuffer {
    void* memory;
    int w;
    int h;
    int pitch;
    int bpp;
};

struct V2 {
    union {
        int w;
        int x;
    };
    union {
        int h;
        int y;
    };

    V2(int a = 0, int b = 0): x{a}, y{b} {}
};

struct V3 {
    union {
        int x;
        int r;
    };
    union {
        int y;
        int g;
    };
    union {
        int z;
        int b;
    };

    V3(int a = 0, int b = 0, int c = 0): x{a}, y{b}, z{c} {}
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

auto constexpr rows = 22;
auto constexpr columns = 10;

auto constexpr baseWindowWidth = columns + 2 + 5; // space for border and sidebar
auto constexpr baseWindowHeight = rows + 2;

auto run() -> void;
