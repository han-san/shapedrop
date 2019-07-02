#include <cassert>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <array>
#include <random>

#include <SDL2/SDL.h>

#include "jint.h"
#include "core.hpp"
#include "platform.hpp"

using Position = V2;
using Color = V3;

struct Block {
    Color color;
    bool isActive = false;
};

auto gRunning = true;

using Board = std::array<Block, rows * columns>;

auto is_valid_spot(Board& board, V2 pos) -> bool {
    if (pos.x < 0 || pos.x >= columns || pos.y < 0 || pos.y >= rows) {
        return false;
    } else {
        auto index = pos.y * columns + pos.x;
        return !board[index].isActive;
    }
}

template <typename T, size_t I>
class ArrayStack {
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const pointer;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    auto constexpr begin() noexcept -> iterator { return &m_data[0]; }
    auto constexpr begin() const noexcept -> const_iterator { return &m_data[0]; }
    auto constexpr end() noexcept -> iterator { return begin() + m_size; }
    auto constexpr end() const noexcept -> const_iterator { return begin() + m_size; }
    auto constexpr cbegin() const noexcept -> const_iterator { return &m_data[0]; }
    auto constexpr cend() const noexcept -> const_iterator { return cbegin() + m_size; }
    auto constexpr front() -> reference { return m_data[0]; }
    auto constexpr front() const -> const_reference { return m_data[0]; }
    auto constexpr back() -> reference { return m_data[m_size - 1]; }
    auto constexpr back() const -> const_reference { return m_data[m_size - 1]; }
    auto constexpr size() noexcept -> size_type { return m_size; }
    auto constexpr max_size() noexcept -> size_type { return I; }
    auto constexpr push_back(value_type i) -> void { m_data[m_size++] = i; }
    auto constexpr pop_back() -> void { m_data[--m_size].~T(); }
    auto constexpr empty() -> bool { return !m_size; }

private:
    std::array<value_type, I> m_data = {};
    size_type m_size = 0;
};

struct Shape {
    using ShapeLayout = std::array<bool, 16>;
    using RotationMap = std::array<ShapeLayout, 4>;

    RotationMap static constexpr IRotationMap = {
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 1, 1,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 1, 0,
            0, 0, 1, 0,
            0, 0, 1, 0,
            0, 0, 1, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            0, 0, 0, 0,
            1, 1, 1, 1,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
        },
    };
    RotationMap static constexpr LRotationMap = {
        ShapeLayout {
            0, 0, 1, 0,
            1, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 1, 0,
            1, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            1, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr JRotationMap = {
        ShapeLayout {
            1, 0, 0, 0,
            1, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 1, 0,
            0, 0, 1, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 0, 0,
            1, 1, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr ORotationMap = {
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr SRotationMap = {
        ShapeLayout {
            0, 1, 1, 0,
            1, 1, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 1, 0,
            0, 0, 1, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            0, 1, 1, 0,
            1, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            1, 0, 0, 0,
            1, 1, 0, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr ZRotationMap = {
        ShapeLayout {
            1, 1, 0, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 1, 0,
            0, 1, 1, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 0, 0,
            0, 1, 1, 1,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            1, 1, 0, 0,
            1, 0, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr TRotationMap = {
        ShapeLayout {
            0, 1, 0, 0,
            1, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 1, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 1, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            1, 1, 0, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
    };

    enum class Type {
        I, O, L, J, S, Z, T
    };

    Type type;
    RotationMap const* rotations = nullptr;
    int rotationIndex = 0;
    Position pos = columns / 2 - 2; // spawn centrally
    Color color;

    Shape(Type type) {
        this->type = type;
        switch (type) {
            case Type::I: {
                color = Color(0x00, 0xf0, 0xf0);
                rotations = &IRotationMap;
            } break;
            case Type::O: {
                color = Color(0xf0, 0xf0, 0x00);
                rotations = &ORotationMap;
            } break;
            case Type::L: {
                color = Color(0xf0, 0xa0, 0x00);
                rotations = &LRotationMap;
            } break;
            case Type::J: {
                color = Color(0x00, 0x00, 0xf0);
                rotations = &JRotationMap;
            } break;
            case Type::S: {
                color = Color(0x00, 0xf0, 0x00);
                rotations = &SRotationMap;
            } break;
            case Type::Z: {
                color = Color(0xf0, 0x00, 0x00);
                rotations = &ZRotationMap;
            } break;
            case Type::T: {
                color = Color(0xa0, 0x00, 0xf0);
                rotations = &TRotationMap;
            } break;
            default: {
                // shouldn't be possible
                assert(false);
            } break;
        }
    }

    auto get_block_positions() -> ArrayStack<Position, 4> {
        ArrayStack<Position, 4> positions = {};
        auto& layout = (*rotations)[rotationIndex];
        auto constexpr size = 4;
        for (auto y = 0; y < size; ++y) {
            for (auto x = 0; x < size; ++x) {
                auto index = y * size + x;
                if (layout[index]) {
                    positions.push_back({x, y});
                    if (positions.size() == 4) {
                        return positions;
                    }
                }
            }
        }
        assert(false);
    }

    auto get_absolute_block_positions() -> ArrayStack<Position, 4> {
        auto positions = get_block_positions();
        for (auto& position : positions) {
            position.x += pos.x;
            position.y += pos.y;
        }
        return positions;
    }

    auto is_valid(Board& board) -> bool {
        for (auto position : get_absolute_block_positions()) {
            if (!is_valid_spot(board, position)) {
                return false;
            }
        }
        return true;
    }

    enum class Rotation {
        LEFT,
        RIGHT
    };

    auto rotate(Board& board, Rotation dir) -> bool {
        auto rotatingShape = *this;
        rotatingShape.rotationIndex += dir == Rotation::RIGHT ? 1 : -1;
        if (rotatingShape.rotationIndex == -1) rotatingShape.rotationIndex = 3;
        else if (rotatingShape.rotationIndex == 4) rotatingShape.rotationIndex = 0;
        if (rotatingShape.is_valid(board)) {
            *this = rotatingShape;
            return true;
        }

        std::array<V2, 4> kicks = {};

        // do wall kicks to see if valid
        switch (type) {
            case Type::J:
            case Type::L:
            case Type::S:
            case Type::T:
            case Type::Z: {
                switch (rotationIndex) {
                    case 0: {
                        if (dir == Rotation::RIGHT) {
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
                        if (dir == Rotation::RIGHT) {
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
            case Type::I: {
                switch (rotationIndex) {
                    case 0: {
                        if (dir == Rotation::RIGHT) {
                            kicks = { V2 {-2, 0}, {1, 0}, {-2, -1}, {1, 2} };
                        } else {
                            kicks = { V2 {-1, 0}, {2, 0}, {-1, 2}, {2, -1} };
                        }
                    } break;
                    case 1: {
                        if (dir == Rotation::RIGHT) {
                            kicks = { V2 {-1, 0}, {2, 0}, {-1, 2}, {2, -1} };
                        } else {
                            kicks = { V2 {2, 0}, {-1, 0}, {2, 1}, {-1, -2} };
                        }
                    } break;
                    case 2: {
                        if (dir == Rotation::RIGHT) {
                            kicks = { V2 {2, 0}, {-1, 0}, {2, 1}, {-1, -2} };
                        } else {
                            kicks = { V2 {1, 0}, {-2, 0}, {1, -2}, {-2, 1} };
                        }
                    } break;
                    case 3: {
                        if (dir == Rotation::RIGHT) {
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
            case Type::O: {
                // should have already returned true in the is_valid() check
                assert(false);
            } break;
            default: {
                assert(false);
            }
        }

        for (auto kickMove : kicks) {
            rotatingShape.pos = pos;
            rotatingShape.pos.x += kickMove.x;
            // the y in kicks is bottom up while it's top down for the shape position
            // so we have to invert it by subtracting instead of adding
            rotatingShape.pos.y -= kickMove.y;
            if (rotatingShape.is_valid(board)) {
                *this = rotatingShape;
                return true;
            }
        }
        return false;
    }
};

auto is_valid_move(Board& board, Shape& shape, V2 move) -> bool {
    shape.pos.x += move.x;
    shape.pos.y += move.y;
    auto valid = shape.is_valid(board);
    shape.pos.x -= move.x;
    shape.pos.y -= move.y;
    return valid;
}

auto try_move(Board& board, Shape& shape, V2 move) {
    if (is_valid_move(board, shape, move)) {
        shape.pos.x += move.x;
        shape.pos.y += move.y;
        return true;
    }
    return false;
}

struct Square {
    float x;
    float y;
    int w;
    int h;
};

struct Point {
    float x;
    float y;
};

auto draw_solid_square(BackBuffer* buf, Square sqr, uint r, uint g, uint b, uint a = 0xff)
{
    for (auto y = 0; y < sqr.h; ++y) {
        auto pixely = (int)sqr.y + y;
        if (pixely < 0 || pixely >= buf->h) {
            continue;
        }
        for (auto x = 0; x < sqr.w; ++x) {
            auto pixelx = (int)sqr.x + x;
            if (pixelx < 0 || pixelx >= buf->w) {
                continue;
            }

            auto currbyteindex = pixely * buf->w + pixelx;
            auto currbyte = ((u8*)buf->memory + currbyteindex * buf->bpp);

            auto alpha_blend = [](uint bg, uint fg, uint alpha) {
                auto alphaRatio = alpha / 255.0;
                return fg * alphaRatio + bg * (1 - alphaRatio);
            };

            *currbyte = alpha_blend(*currbyte, b, a);
            ++currbyte;
            *currbyte = alpha_blend(*currbyte, g, a);
            ++currbyte;
            *currbyte = alpha_blend(*currbyte, r, a);
        }
    }
}

auto draw_image(BackBuffer* backBuf, Point dest, BackBuffer* img)
{
    for (auto y = 0; y < img->h; ++y) {
        auto pixely = (int)dest.y + y;
        if (pixely < 0 || pixely >= backBuf->h) {
            continue;
        }
        for (auto x = 0; x < img->w; ++x) {
            auto pixelx = (int)dest.x + x;
            if (pixelx < 0 || pixelx >= backBuf->w) {
                continue;
            }

            auto currBBbyteindex = pixely * backBuf->w + pixelx;
            auto currBBbyte = ((u8*)backBuf->memory + currBBbyteindex * backBuf->bpp);
            auto currimgbyteindex = y * img->w + x;
            auto currimgbyte = ((u8*)img->memory + currimgbyteindex * img->bpp);

            auto r = *currimgbyte++;
            auto g = *currimgbyte++;
            auto b = *currimgbyte++;
            auto a = *currimgbyte++;

            // FIXME: hack
            if (!a) {
                continue;
            }

            *currBBbyte++ = b;
            *currBBbyte++ = g;
            *currBBbyte++ = r;
        }
    }
}

auto currentclock = (decltype(clock())) 0;
auto dropclock = (decltype(clock())) 0;
auto constexpr lockdelay = (decltype(clock())) CLOCKS_PER_SEC / 2;
auto lockclock = (decltype(clock())) 0;

auto highScore = 0;

auto dropSpeed = 1.0;
auto maxDropSpeed = 0.1;

auto init()
{
    currentclock = clock();
    dropclock = currentclock;
    lockclock = currentclock;

    srand(time(NULL));
}

auto remove_full_rows(std::array<Block, rows * columns>& board) {
    // check if a row can be cleared
    // a maximum of 4 rows can be cleared at once with default shapes
    ArrayStack<int, 4> rowsCleared;

    for (auto y = 0; y < rows; ++y) {
        auto rowIsFull = true;
        for (auto x = 0; x < columns; ++x) {
            auto boardIndex = y * columns + x;
            if (!board[boardIndex].isActive) {
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
                board[index].isActive = false;
            }
        }

        auto move_row_down = [&board](int rowNumber, int distance) {
            assert(distance > 0);
            for (auto x = 0; x < columns; ++x) {
                auto index = rowNumber * columns + x;
                auto newIndex = (rowNumber + distance) * columns + x;
                assert((rowNumber + distance) < rows);
                auto& oldBlock = board[index];
                auto& newBlock = board[newIndex];
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

auto print_board(std::array<Block, rows * columns>& board) {
    std::cout << ' ';
    for (auto i = 0; i < columns; ++i) std::cout << "_";
    std::cout << '\n';
    for (auto y = 0; y < rows; ++y) {
        std::cout << '|';
        for (auto x = 0; x < columns; ++x) {
            auto index = y * columns + x;
            auto currBlock = board[index];

            std::cout << (currBlock.isActive ? "O" : " ");
        }
        std::cout << "|\n";
    }
    std::cout << '|';
    for (auto i = 0; i < columns; ++i) std::cout << "-";
    std::cout << "|\n";
}

namespace tests {
auto remove_full_rows() {
    auto y = Block { {}, true };
    auto n = Block { {}, false };
    std::array<Block, rows * columns> boardStart = {
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, y, y, y, n, n, n,
        n, n, n, y, y, n, y, n, n, n,
        n, y, y, y, y, n, n, n, n, n,
        n, y, n, n, y, n, n, n, n, n,
        y, y, y, y, y, y, y, y, y, y,
        y, n, y, n, y, n, y, n, y, n,
        n, y, n, y, n, y, n, y, n, y,
        y, y, y, y, y, y, y, y, y, y,
    };

    std::array<Block, rows * columns> boardEnd = {
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, n, n, n, n, n, n,
        n, n, n, n, y, y, y, n, n, n,
        n, n, n, y, y, n, y, n, n, n,
        n, y, y, y, y, n, n, n, n, n,
        n, y, n, n, y, n, n, n, n, n,
        y, n, y, n, y, n, y, n, y, n,
        n, y, n, y, n, y, n, y, n, y,
    };

    remove_full_rows(boardStart);

    for (auto i = 0ul; i < boardStart.size(); ++i) {
        assert(boardStart[i].isActive == boardEnd[i].isActive);
    }
}

auto run() {
    remove_full_rows();
}
} // namespace test

auto run() -> void
{
    tests::run();
    gRunning = true;

    init();

    Board board = {};

    std::array<Shape, 7> const shapes = {
        Shape(Shape::Type::I),
        Shape(Shape::Type::L),
        Shape(Shape::Type::J),
        Shape(Shape::Type::O),
        Shape(Shape::Type::S),
        Shape(Shape::Type::Z),
        Shape(Shape::Type::T),
    };

    class ShapePool {
        std::array<const Shape*, 7> shapePool;
        decltype(shapePool) previewPool;
        decltype(shapePool.begin()) currentShapeIterator;

    public:
        ShapePool(const std::array<Shape, 7>& shapes) {
            shapePool = {
                &shapes[0], &shapes[1], &shapes[2],
                &shapes[3], &shapes[4], &shapes[5],
                &shapes[6],
            };
            previewPool = shapePool;

            // TODO: seed random engine
            std::shuffle(shapePool.begin(), shapePool.end(), std::default_random_engine());
            std::shuffle(previewPool.begin(), previewPool.end(), std::default_random_engine());
            currentShapeIterator = shapePool.begin();
        }

        auto next_shape() -> Shape {
            ++currentShapeIterator;
            if (currentShapeIterator == shapePool.end()) {
                shapePool = previewPool;
                currentShapeIterator = shapePool.begin();
                std::shuffle(previewPool.begin(), previewPool.end(), std::default_random_engine());
            }
            return **currentShapeIterator;
        }

        auto current_shape() -> Shape {
            return **currentShapeIterator;
        }

        auto get_preview_shapes_array() -> ArrayStack<Shape const*, 14> {
            ArrayStack<Shape const*, 14> lookaheadArray = {};
            for (auto it = currentShapeIterator + 1; it != shapePool.end(); ++it) {
                lookaheadArray.push_back(*it);
            }
            for (auto it = previewPool.cbegin(); it != previewPool.cend(); ++it) {
                lookaheadArray.push_back(*it);
            }
            return lookaheadArray;
        }
    };
    ShapePool shapePool{shapes};

    auto calculateShapeShadow = [&board](Shape& shape) {
        auto shapeShadow = shape;
        while (is_valid_move(board, shapeShadow, {0, 1})) {
            ++shapeShadow.pos.y;
        }
        return shapeShadow;
    };

    auto currentShape = shapePool.current_shape();
    auto currentShapeShadow = calculateShapeShadow(currentShape);

    while (gRunning) {
        auto newclock = clock();
        auto frameclocktime = newclock - currentclock;
        currentclock = newclock;

        // delta = (double)frameclocktime / CLOCKS_PER_SEC;
        /* auto framemstime = 1000.0 * delta; */

        // TODO: sleep so cpu doesn't melt

        // input
        Message message;
        while ((message = handle_input()) != Message::NONE) {
            if (message == Message::QUIT) {
                gRunning = false;
            } else if (message == Message::RESET) {
                /* init(); */
            } else if (message == Message::MOVE_RIGHT) {
                // if currentShape is on top of a block before move,
                // the drop clock needs to be reset
                auto isGrounded = !is_valid_move(board, currentShape, {0, 1});
                if (try_move(board, currentShape, {1, 0})) {
                    currentShapeShadow = calculateShapeShadow(currentShape);
                    lockclock = currentclock;
                    if (isGrounded) {
                        dropclock = currentclock;
                    }
                }
            } else if (message == Message::MOVE_LEFT) {
                // if currentShape is on top of a block before move,
                // the drop clock needs to be reset
                auto isGrounded = !is_valid_move(board, currentShape, {0, 1});
                if (try_move(board, currentShape, {-1, 0})) {
                    currentShapeShadow = calculateShapeShadow(currentShape);
                    lockclock = currentclock;
                    if (isGrounded) {
                        dropclock = currentclock;
                    }
                }
            } else if (message == Message::INCREASE_WINDOW_SIZE) {
                change_window_scale(get_window_scale() + 1);
            } else if (message == Message::DECREASE_WINDOW_SIZE) {
                change_window_scale(get_window_scale() - 1);
            } else if (message == Message::INCREASE_SPEED) {
                dropSpeed = maxDropSpeed;
            } else if (message == Message::RESET_SPEED) {
                dropSpeed = 1.0;
            } else if (message == Message::DROP) {
                while (try_move(board, currentShape, {0, 1})) {
                    lockclock = currentclock;
                }
            } else if (message == Message::ROTATE_LEFT) {
                // if currentShape is on top of a block before rotation,
                // the drop clock needs to be reset
                auto isGrounded = !is_valid_move(board, currentShape, {0, 1});
                if (currentShape.rotate(board, Shape::Rotation::LEFT)) {
                    currentShapeShadow = calculateShapeShadow(currentShape);
                    lockclock = currentclock;
                    if (isGrounded) {
                        dropclock = currentclock;
                    }
                }
            } else if (message == Message::ROTATE_RIGHT) {
                // if currentShape is on top of a block before rotation,
                // the drop clock needs to be reset
                auto isGrounded = !is_valid_move(board, currentShape, {0, 1});
                if (currentShape.rotate(board, Shape::Rotation::RIGHT)) {
                    currentShapeShadow = calculateShapeShadow(currentShape);
                    lockclock = currentclock;
                    if (isGrounded) {
                        dropclock = currentclock;
                    }
                }
            }
        }

        // sim
        {
            // 1 drop per second
            auto nextdropclock = dropclock + dropSpeed * CLOCKS_PER_SEC;
            if (currentclock > nextdropclock) {
                dropclock = currentclock;
                if (try_move(board, currentShape, {0, 1})) {
                    lockclock = currentclock;
                }
            }

            if (currentclock > lockclock + lockdelay) {
                // only care about locking if currentShape is on top of a block
                if (!is_valid_move(board, currentShape, {0, 1})) {
                    // game over if entire piece is above visible portion
                    // of board
                    auto gameOver = true;
                    for (auto pos : currentShape.get_absolute_block_positions()) {
                        if (pos.y > 1) {
                            gameOver = false;
                        }
                    }

                    // fix currentBlocks position on board
                    for (auto position : currentShape.get_absolute_block_positions()) {
                        assert(is_valid_spot(board, position));
                        auto boardIndex = position.y * columns + position.x;
                        board[boardIndex] = {currentShape.color, true};
                    }

                    remove_full_rows(board);
                    currentShape = shapePool.next_shape();
                    // update shape shadow
                    currentShapeShadow = calculateShapeShadow(currentShape);
                    lockclock = currentclock;

                    // game over if the new shape spawned on top of another shape
                    if (!currentShape.is_valid(board)) {
                        gameOver = true;
                    }

                    if (gameOver) {
                        std::cout << "Game Over!\n";
                        gRunning = false;
                    }
                }
            }
        }

        // draw border
        auto windim = get_window_dimensions();
        auto bb = get_back_buffer();
        auto scale = get_window_scale();
        for (auto y = 0; y < windim.h; ++y) {
            for (auto x = 0; x < windim.w; ++x) {
                draw_solid_square(&bb, {float(x), float(y), 1, 1}, 0xff * (float(x) / windim.w), 0xff * (1 - (float(x) / windim.w) * (float(y) / windim.h)), 0xff * (float(y) / windim.h));
            }
        }

        // draw background
        for (auto y = 2; y < rows; ++y) {
            for (auto x = 0; x < columns; ++x) {
                auto currindex = y * columns + x;
                auto& block = board[currindex];
                auto color = block.isActive ? block.color : Color { 0, 0, 0 };
                draw_solid_square(&bb, {float((x + 1) * scale), float((y + 1) * scale), scale, scale}, color.r, color.g, color.b);
            }
        }

        // draw shadow
        for (auto& position : currentShapeShadow.get_absolute_block_positions()) {
            draw_solid_square(&bb, {float((position.x + 1) * scale), float((position.y + 1) * scale), scale, scale}, currentShapeShadow.color.r, currentShapeShadow.color.g, currentShapeShadow.color.b, 0xff / 2);
        }

        // draw current shape
        for (auto& position : currentShape.get_absolute_block_positions()) {
            draw_solid_square(&bb, {float((position.x + 1) * scale), float((position.y + 1) * scale), scale, scale}, currentShape.color.r, currentShape.color.g, currentShape.color.b);
        }

        // draw shape previews
        auto previewArray = shapePool.get_preview_shapes_array();
        auto i = 0;
        for (auto shapePointer : previewArray) {
            auto shape = *shapePointer;
            shape.pos.x = baseWindowWidth - 6;
            shape.pos.y += 3 + 3 * i;
            for (auto& position : shape.get_absolute_block_positions()) {
                draw_solid_square(&bb, {float((position.x + 1) * scale), float((position.y + 1) * scale), scale, scale}, shape.color.r, shape.color.g, shape.color.b);
            }
            ++i;
        }

        // cover top part of border
        auto topSize = scale * 3;
        for (auto y = 0; y < topSize; ++y) {
            for (auto x = 0; x < windim.w; ++x) {
                draw_solid_square(&bb, {float(x), float(y), 1, 1}, 0xff * (float(x) / windim.w), 0xff * (1 - (float(x) / windim.w) * (float(y) / windim.h)), 0xff * (float(y) / windim.h));
            }
        }

        swap_buffer();
    }
}
