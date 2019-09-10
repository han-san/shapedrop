#include <cassert>
#include <iostream>

#include "board.hpp"

auto Board::is_valid_spot(V2 pos) -> bool {
    if (pos.x < 0 || pos.x >= columns || pos.y < 0 || pos.y >= rows) {
        return false;
    } else {
        auto index = pos.y * columns + pos.x;
        return !data[index].isActive;
    }
}

auto Board::is_valid_move(Shape& shape, V2 move) -> bool {
    shape.pos.x += move.x;
    shape.pos.y += move.y;
    auto valid = is_valid_shape(shape);
    shape.pos.x -= move.x;
    shape.pos.y -= move.y;
    return valid;
}

auto Board::is_valid_shape(Shape& shape) -> bool {
    for (auto position : shape.get_absolute_block_positions()) {
        if (!is_valid_spot(position)) {
            return false;
        }
    }
    return true;
}

auto Board::remove_full_rows() -> void {
    // check if a row can be cleared
    // a maximum of 4 rows can be cleared at once with default shapes
    ArrayStack<int, 4> rowsCleared;

    for (auto y = 0; y < rows; ++y) {
        auto rowIsFull = true;
        for (auto x = 0; x < columns; ++x) {
            auto boardIndex = y * columns + x;
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
        for (auto y : rowsCleared) {
            for (auto x = 0; x < columns; ++x) {
                auto index = y * columns + x;
                data[index].isActive = false;
            }
        }

        auto move_row_down = [this](int rowNumber, int distance) {
            assert(distance > 0);
            for (auto x = 0; x < columns; ++x) {
                auto index = rowNumber * columns + x;
                auto newIndex = (rowNumber + distance) * columns + x;
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
            auto topRow = rowsCleared.front();
            auto botRow = rowsCleared.back();
            assert(topRow >= 0 && topRow < rows);
            assert(botRow >= 0 && botRow < rows);
            assert(botRow >= topRow);

            auto emptyRowsPassed = 0;
            for (auto y = botRow; y != topRow; --y) {
                auto found = false;
                for (auto row : rowsCleared) {
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
}

auto Board::print_board() -> void {
    std::cout << ' ';
    for (auto i = 0; i < columns; ++i) std::cout << "_";
    std::cout << '\n';
    for (auto y = 0; y < rows; ++y) {
        std::cout << '|';
        for (auto x = 0; x < columns; ++x) {
            auto index = y * columns + x;
            auto currBlock = data[index];

            std::cout << (currBlock.isActive ? "O" : " ");
        }
        std::cout << "|\n";
    }
    std::cout << '|';
    for (auto i = 0; i < columns; ++i) std::cout << "-";
    std::cout << "|\n";
}
