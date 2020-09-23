#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>

#include "board.hpp"

auto Board::get_shadow(Shape const& shape) const -> Shape {
    auto shapeShadow {shape};
    while (is_valid_move(shapeShadow, {0, 1})) {
        ++shapeShadow.pos.y;
    }

    shapeShadow.color.a = Color::RGBA::Alpha::opaque / 2;
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
    if (pos.x < 0 || pos.x >= columns || pos.y < 0 || pos.y >= rows) {
        return false;
    } else {
        auto const index {pos.y * columns + pos.x};
        return !data[index].isActive;
    }
}

auto Board::is_valid_move(Shape shape, V2 const move) const -> bool {
    shape.pos += move;
    return is_valid_shape(shape);
}

auto Board::is_valid_shape(Shape const& shape) const -> bool {
    auto const ShapePositions {shape.get_absolute_block_positions()};
    return std::all_of(std::cbegin(ShapePositions), std::cend(ShapePositions), [this](auto const& position) {
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
            std::count_if(std::cbegin(cornerOffsets), std::cend(cornerOffsets), [this, &shape](auto const& offset) {
                          return !is_valid_spot(shape.pos + offset);
                          })
        };
        if (cornersOccupied >= 3) {
            return (rotationType == Shape::RotationType::Wallkick) ? TspinType::Mini : TspinType::Regular;
        }
    }
    return std::nullopt;
}

auto Board::remove_full_rows() -> int {
    // check if a row can be cleared
    // a maximum of 4 rows can be cleared at once with default shapes
    ArrayStack<int, 4> rowsCleared;

    for (auto y {0}; y < rows; ++y) {
        auto rowIsFull {true};
        for (auto x {0}; x < columns; ++x) {
            auto const boardIndex {y * columns + x};
            if (!data[boardIndex].isActive) {
                rowIsFull = false;
                break;
            }
        }
        if (rowIsFull) {
            rowsCleared.push_back(y);
        }
    }
    assert(rowsCleared.size() <= 4);

    if (rowsCleared.empty()) {
        return 0;
    }

    auto move_row_down = [this](int const rowNumber, int const distance) {
        assert(distance > 0);
        for (auto x {0}; x < columns; ++x) {
            auto const index {rowNumber * columns + x};
            auto const newIndex {(rowNumber + distance) * columns + x};
            assert((rowNumber + distance) < rows);
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
        assert(topRow >= 0 && topRow < rows);
        assert(botRow >= 0 && botRow < rows);
        assert(botRow >= topRow);

        auto emptyRowsPassed {0};
        for (auto y {botRow}; y != topRow; --y) {
            auto const found {
                std::any_of(std::cbegin(rowsCleared), std::cend(rowsCleared), [y](auto const& row) {
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
    for (auto y {rowsCleared.front() - 1}; y >= 0; --y) {
        move_row_down(y, rowsCleared.size());
    }

    return rowsCleared.size();
}

auto Board::print_board() const -> void {
    std::cout << ' ';
    for (auto i {0}; i < columns; ++i) std::cout << "_";
    std::cout << '\n';
    for (auto y {0}; y < rows; ++y) {
        std::cout << '|';
        for (auto x {0}; x < columns; ++x) {
            auto const index {y * columns + x};
            auto const currBlock {data[index]};

            std::cout << (currBlock.isActive ? "O" : " ");
        }
        std::cout << "|\n";
    }
    std::cout << '|';
    for (auto i {0}; i < columns; ++i) std::cout << "-";
    std::cout << "|\n";
}
