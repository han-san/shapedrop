#pragma once

#include <array>
#include <optional>

#include "jint.h"

#include "shape.hpp"
#include "util.hpp"

struct Block {
    Color::RGBA color {Color::invalid};
    bool isActive {false};
};

enum class TspinType {
    Regular,
    Mini
};

class Board {
public:
    int static constexpr rows {22};
    int static constexpr columns {10};
    int static constexpr visibleRows {rows - 2};

    std::array<Block, rows * columns> data;

    auto rotate_shape(Shape& shape, Shape::RotationDirection dir) const -> std::optional<Shape::RotationType>;
    auto try_move(Shape& shape, V2 move) const -> bool;
    auto get_shadow(Shape const& shape) const -> Shape;
    auto check_for_tspin(Shape const& shape, Shape::RotationType rotationType) const -> std::optional<TspinType>;
    auto is_valid_spot(Position pos) const -> bool;
    auto is_valid_move(Shape shape, V2 move) const -> bool;
    auto is_valid_shape(Shape const& shape) const -> bool;
    auto remove_full_rows() -> int;
    auto print_board() const -> void;
};
