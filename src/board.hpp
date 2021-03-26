#pragma once

#include "jint.h"
#include "shape.hpp"
#include "util.hpp"

#include <array>
#include <optional>

struct Block {
    Color::RGBA color = Color::invalid;
    bool isActive {false};
};

enum class TspinType {
    Regular,
    Mini
};

class Board {
public:
    u8 static constexpr rows {22};
    u8 static constexpr columns {10};
    u8 static constexpr visibleRows {rows - 2};

    std::array<Block, rows * columns> data {make_filled_array<Block, rows * columns>({Color::black, false})};

    auto rotate_shape(Shape& shape, Shape::RotationDirection dir) const -> std::optional<Shape::RotationType>;
    auto try_move(Shape& shape, V2 move) const -> bool;
    [[nodiscard]] auto get_shadow(Shape const& shape) const -> Shape;
    [[nodiscard]] auto check_for_tspin(Shape const& shape, Shape::RotationType rotationType) const -> std::optional<TspinType>;
    [[nodiscard]] auto is_valid_spot(Point<int> pos) const -> bool;
    [[nodiscard]] auto is_valid_move(Shape shape, V2 move) const -> bool;
    [[nodiscard]] auto is_valid_shape(Shape const& shape) const -> bool;
    auto remove_full_rows() -> u8;
    auto print_board() const -> void;
private:
    [[nodiscard]] auto get_cleared_rows() const -> ArrayStack<u8, Shape::maxHeight>;
};
