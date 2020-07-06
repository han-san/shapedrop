#include <cassert>
#include <iostream>

#include "board.hpp"

auto Board::get_shadow(Shape const& shape) const -> Shape {
    auto shapeShadow = shape;
    while (is_valid_move(shapeShadow, {0, 1})) {
        ++shapeShadow.pos.y;
    }
    return shapeShadow;
}

auto Board::try_move(Shape& shape, V2 const move) const -> bool {
    if (is_valid_move(shape, move)) {
        shape.pos.x += move.x;
        shape.pos.y += move.y;
        return true;
    }
    return false;
}

auto Board::rotate_shape(Shape& shape, Shape::Rotation const dir) const -> std::optional<Shape::RotationType> {
    auto rotatingShape = shape;
    rotatingShape.rotationIndex += dir == Shape::Rotation::RIGHT ? 1 : -1;
    if (rotatingShape.rotationIndex == -1) rotatingShape.rotationIndex = 3;
    else if (rotatingShape.rotationIndex == 4) rotatingShape.rotationIndex = 0;
    if (is_valid_shape(rotatingShape)) {
        // only rotationindex should have changed
        shape = rotatingShape;
        return Shape::RotationType::REGULAR;
    }

    std::array<V2, 4> kicks = {};

    // do wall kicks to see if valid
    switch (shape.type) {
        case Shape::Type::J:
        case Shape::Type::L:
        case Shape::Type::S:
        case Shape::Type::T:
        case Shape::Type::Z: {
            switch (shape.rotationIndex) {
                case 0: {
                    if (dir == Shape::Rotation::RIGHT) {
                        kicks = { V2 {-1, 0}, {-1, 1}, {0, -2}, {-1, -2} };
                    } else {
                        kicks = { V2 {1, 0}, {1, 1}, {0, -2}, {1, -2} };
                    }
                } break;
                case 1: {
                    // both directions check same positions
                    kicks = { V2 {1, 0}, {1, -1}, {0, 2}, {1, 2} };
                } break;
                case 2: {
                    if (dir == Shape::Rotation::RIGHT) {
                        kicks = { V2 {1, 0}, {1, 1}, {0, -2}, {1, -2} };
                    } else {
                        kicks = { V2 {-1, 0}, {-1, 1}, {0, -2}, {-1, -2} };
                    }
                } break;
                case 3: {
                    // both directions check same positions
                    kicks = { V2 {-1, 0}, {-1, -1}, {0, 2}, {-1, 2} };
                } break;
                default: {
                    assert(false);
                } break;
            }
        } break;
        case Shape::Type::I: {
            switch (shape.rotationIndex) {
                case 0: {
                    if (dir == Shape::Rotation::RIGHT) {
                        kicks = { V2 {-2, 0}, {1, 0}, {-2, -1}, {1, 2} };
                    } else {
                        kicks = { V2 {-1, 0}, {2, 0}, {-1, 2}, {2, -1} };
                    }
                } break;
                case 1: {
                    if (dir == Shape::Rotation::RIGHT) {
                        kicks = { V2 {-1, 0}, {2, 0}, {-1, 2}, {2, -1} };
                    } else {
                        kicks = { V2 {2, 0}, {-1, 0}, {2, 1}, {-1, -2} };
                    }
                } break;
                case 2: {
                    if (dir == Shape::Rotation::RIGHT) {
                        kicks = { V2 {2, 0}, {-1, 0}, {2, 1}, {-1, -2} };
                    } else {
                        kicks = { V2 {1, 0}, {-2, 0}, {1, -2}, {-2, 1} };
                    }
                } break;
                case 3: {
                    if (dir == Shape::Rotation::RIGHT) {
                        kicks = { V2 {1, 0}, {-2, 0}, {1, -2}, {-2, 1} };
                    } else {
                        kicks = { V2 {-2, 0}, {1, 0}, {-2, -1}, {1, 2} };
                    }
                } break;
                default: {
                    assert(false);
                } break;
            }
        } break;
        case Shape::Type::O: {
            // should have already returned true in the is_valid() check
            assert(false);
        } break;
        default: {
            assert(false);
        }
    }

    for (auto const kickMove : kicks) {
        rotatingShape.pos = shape.pos;
        rotatingShape.pos.x += kickMove.x;
        // the y in kicks is bottom up while it's top down for the shape position
        // so we have to invert it by subtracting instead of adding
        rotatingShape.pos.y -= kickMove.y;
        if (is_valid_shape(rotatingShape)) {
            shape = rotatingShape;
            return Shape::RotationType::WALLKICK;
        }
    }
    return {};
}

auto Board::is_valid_spot(Position const pos) const -> bool {
    if (pos.x < 0 || pos.x >= columns || pos.y < 0 || pos.y >= rows) {
        return false;
    } else {
        auto const index = pos.y * columns + pos.x;
        return !data[index].isActive;
    }
}

auto Board::is_valid_move(Shape shape, V2 const move) const -> bool {
    shape.pos.x += move.x;
    shape.pos.y += move.y;
    return is_valid_shape(shape);
}

auto Board::is_valid_shape(Shape& shape) const -> bool {
    for (auto const position : shape.get_absolute_block_positions()) {
        if (!is_valid_spot(position)) {
            return false;
        }
    }
    return true;
}

// If the shape is a T, its last movement was a rotation, and 3 or more of its
// corners are occupied by other pieces it counts as a T-spin. If the rotation
// was a wallkick it only counts as a T-spin mini.
auto Board::check_for_tspin(Shape const& shape, Shape::RotationType const rotationType) const -> std::optional<TspinType> {
    if (shape.type == Shape::Type::T) {
        V2 const cornerOffsets[] = {{0, 0}, {2, 0}, {0, 2}, {2, 2}};
        auto cornersOccupied = 0;
        for (auto const offset : cornerOffsets) {
            cornersOccupied += !is_valid_spot({shape.pos.x + offset.x, shape.pos.y + offset.y});
        }
        if (cornersOccupied >= 3) {
            return (rotationType == Shape::RotationType::WALLKICK) ? TspinType::MINI : TspinType::REGULAR;
        }
    }
    return {};
}

auto Board::remove_full_rows() -> int {
    // check if a row can be cleared
    // a maximum of 4 rows can be cleared at once with default shapes
    ArrayStack<int, 4> rowsCleared;

    for (auto y = 0; y < rows; ++y) {
        auto rowIsFull = true;
        for (auto x = 0; x < columns; ++x) {
            auto const boardIndex = y * columns + x;
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

    if (!rowsCleared.empty()) {
        // remove rows
        for (auto const y : rowsCleared) {
            for (auto x = 0; x < columns; ++x) {
                auto const index = y * columns + x;
                data[index].isActive = false;
            }
        }

        auto move_row_down = [this](int const rowNumber, int const distance) {
            assert(distance > 0);
            for (auto x = 0; x < columns; ++x) {
                auto const index = rowNumber * columns + x;
                auto const newIndex = (rowNumber + distance) * columns + x;
                assert((rowNumber + distance) < rows);
                auto& oldBlock = this->data[index];
                auto& newBlock = this->data[newIndex];
                newBlock = oldBlock;
                oldBlock.isActive = false;
            }
        };

        // if the amount of rows is 2 or 3, there could be non-full rows
        // between full rows which need to be moved different amounts

        // first move any potential rows between the cleared ones
        if (rowsCleared.size() == 2 || rowsCleared.size() == 3) {
            auto const topRow = rowsCleared.front();
            auto const botRow = rowsCleared.back();
            assert(topRow >= 0 && topRow < rows);
            assert(botRow >= 0 && botRow < rows);
            assert(botRow >= topRow);

            auto emptyRowsPassed = 0;
            for (auto y = botRow; y != topRow; --y) {
                auto found = false;
                for (auto const row : rowsCleared) {
                    if (row == y) {
                        found = true;
                        break;
                    }
                }
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
        for (auto y = rowsCleared.front() - 1; y >= 0; --y) {
            move_row_down(y, rowsCleared.size());
        }
    }

    return rowsCleared.size();
}

auto Board::print_board() const -> void {
    std::cout << ' ';
    for (auto i = 0; i < columns; ++i) std::cout << "_";
    std::cout << '\n';
    for (auto y = 0; y < rows; ++y) {
        std::cout << '|';
        for (auto x = 0; x < columns; ++x) {
            auto const index = y * columns + x;
            auto const currBlock = data[index];

            std::cout << (currBlock.isActive ? "O" : " ");
        }
        std::cout << "|\n";
    }
    std::cout << '|';
    for (auto i = 0; i < columns; ++i) std::cout << "-";
    std::cout << "|\n";
}
