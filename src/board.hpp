#pragma once

#include <array>

#include "util.hpp"
#include "jint.h"
#include "shape.hpp"

class Shape;

struct Block {
    RGB color;
    bool isActive = false;
};

enum class TspinType {
    REGULAR,
    MINI
};

class Board {
public:
    int static constexpr rows = 22;
    int static constexpr columns = 10;
    int static constexpr visibleRows = rows - 2;

    std::array<Block, rows * columns> data;

    auto check_for_tspin(Shape& shape, Shape::RotationType rotationType) -> std::optional<TspinType>;
    auto is_valid_spot(Position pos) -> bool;
    auto is_valid_move(Shape& shape, V2 move) -> bool;
    auto is_valid_shape(Shape& shape) -> bool;
    auto remove_full_rows() -> int;
    auto print_board() -> void;
};
