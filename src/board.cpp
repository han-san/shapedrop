#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>

#include "jint.h"
#include "rangealgorithms.hpp"

#include "board.hpp"

auto Board::get_shadow(Shape const& shape) const -> Shape {
    auto shapeShadow {shape};
    while (try_move(shapeShadow, V2::down())) {
        // Intentionally empty body.
    }
    shapeShadow.color.a = Color::RGBA::Alpha::opaque / 2U;
    return shapeShadow;
}

auto Board::try_move(Shape& shape, V2 const move) const -> bool {
    if (is_valid_move(shape, move)) {
        shape.pos += move;
        return true;
    }
    return false;
}

auto Board::rotate_shape(Shape& shape, Shape::RotationDirection const dir) const -> std::optional<Shape::RotationType> {
    auto rotatingShape {shape};
    rotatingShape.rotation += dir;
    if (is_valid_shape(rotatingShape)) {
        shape.rotation = rotatingShape.rotation;
        return Shape::RotationType::Regular;
    }

    // Something is blocking the shape after just rotating it, so it has to be
    // kicked into a valid position if possible.
    for (auto const kickMove : shape.get_wallkicks(dir)) {
        // rotatingShape already has the new rotation, but has to reset its position every time it checks a new kick.
        rotatingShape.pos = shape.pos;
        rotatingShape.pos.x += kickMove.x;
        // the y in kicks is bottom up while it's top down for the shape position
        // so we have to invert it by subtracting instead of adding
        rotatingShape.pos.y -= kickMove.y;
        if (is_valid_shape(rotatingShape)) {
            shape = rotatingShape;
            return Shape::RotationType::Wallkick;
        }
    }
    return std::nullopt;
}

auto Board::is_valid_spot(Point<int> const pos) const -> bool {
    if (point_is_in_rect(pos, {0, 0, columns, rows})) {
        auto const index {static_cast<std::size_t>(pos.y * columns + pos.x)};
        return !data[index].isActive;
    }
    return false;
}

auto Board::is_valid_move(Shape shape, V2 const move) const -> bool {
    shape.pos += move;
    return is_valid_shape(shape);
}

auto Board::is_valid_shape(Shape const& shape) const -> bool {
    auto const blockPositions {shape.get_absolute_block_positions()};
    return all_of(blockPositions, [this](auto const& position) {
                  return is_valid_spot(position);
                  });
}

// If the shape is a T, its last movement was a rotation, and 3 or more of its
// corners are occupied by other pieces it counts as a T-spin. If the rotation
// was a wallkick it only counts as a T-spin mini.
auto Board::check_for_tspin(Shape const& shape, Shape::RotationType const rotationType) const -> std::optional<TspinType> {
    if (shape.type == Shape::Type::T) {
        std::array<V2, 4> static constexpr cornerOffsets {V2 {0, 0}, {2, 0}, {0, 2}, {2, 2}};
        auto const cornersOccupied {
            count_if(cornerOffsets, [this, &shape](auto const& offset) {
                     return !is_valid_spot(shape.pos + offset);
                     })
        };
        if (cornersOccupied >= 3) {
            return (rotationType == Shape::RotationType::Wallkick) ? TspinType::Mini : TspinType::Regular;
        }
    }
    return std::nullopt;
}

auto Board::get_cleared_rows() const -> ArrayStack<u8, Shape::maxHeight> {
    ArrayStack<u8, Shape::maxHeight> rowsCleared;

    for (u8 y {0}; y < rows; ++y) {
        auto const rowStartIt {data.cbegin() + (y * columns)};
        auto const rowEndIt {data.cbegin() + ((y + 1) * columns)};
        auto const rowIsFull {
            std::all_of(rowStartIt, rowEndIt, [](auto const& block) {
                        return block.isActive;
                        })
        };
        if (rowIsFull) {
            rowsCleared.push_back(y);
        }
    }

    return rowsCleared;
}

auto Board::remove_full_rows() -> u8 {
    auto const rowsCleared {get_cleared_rows()};

    if (rowsCleared.empty()) {
        return 0;
    }

    auto move_row_down = [this](PositiveSize_t const rowNumber, PositiveSize_t const distance) {
        assert(std::size_t {distance} != 0);
        assert(std::size_t {rowNumber} + std::size_t {distance} < rows);
        for (u8 x {0}; x < columns; ++x) {
            std::size_t const index {rowNumber * columns + x};
            std::size_t const newIndex {(rowNumber + distance) * columns + x};
            auto& oldBlock {this->data[index]};
            auto& newBlock {this->data[newIndex]};
            newBlock = oldBlock;
            oldBlock.isActive = false;
            oldBlock.color = Color::black;
        }
    };

    // if the amount of rows is 2 or 3, there could be non-full rows
    // between full rows which need to be moved different amounts

    // first move any potential rows between the cleared ones
    if (rowsCleared.size() == 2 || rowsCleared.size() == 3) {
        auto const topRow {rowsCleared.front()};
        auto const botRow {rowsCleared.back()};
        assert(topRow < rows);
        assert(botRow < rows);
        assert(botRow >= topRow);

        u8 emptyRowsPassed {0};
        for (auto y {botRow}; y != topRow; --y) {
            auto const found {
                any_of(rowsCleared, [y](auto const& row) {
                       return row == y;
                       })
            };
            if (!found) {
                // a non-full row between full rows
                // move down the amount of empty rows that have been passed
                move_row_down(y, emptyRowsPassed);
                emptyRowsPassed = 0;
            }
            ++emptyRowsPassed;
        }
    }

    // then move all rows above removed rows
    auto const rowAboveRemovedRows {PositiveSize_t {rowsCleared.front()} - 1U};
    for (std::size_t y {0}; y <= rowAboveRemovedRows; ++y) {
        move_row_down(rowAboveRemovedRows - y, rowsCleared.size());
    }

    return static_cast<u8>(rowsCleared.size());
}
