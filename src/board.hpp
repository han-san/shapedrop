#pragma once

#include <array>

#include "util.hpp"
#include "jint.h"
#include "shape.hpp"

struct Block {
    Color color;
    bool isActive = false;
};

auto constexpr gRows = 22;
auto constexpr gColumns = 10;

class Shape;

using BoardData = std::array<Block, gRows * gColumns>;

class Board {
public:
    BoardData data;
    int static constexpr rows = gRows;
    int static constexpr columns = gColumns;

    auto is_valid_spot(V2 pos) -> bool;
    auto is_valid_move(Shape& shape, V2 move) -> bool;
    auto is_valid_shape(Shape& shape) -> bool;
    auto remove_full_rows() -> int;
    auto print_board() -> void;
};
