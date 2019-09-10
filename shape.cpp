#include <cassert>

#include "board.hpp"

#include "shape.hpp"

Shape::Shape(Type type, Board& board)
    : pos{board.columns / 2 - 2} // spawn centrally
{
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

auto Shape::get_shadow(Board& board) -> Shape {
    auto shapeShadow = *this;
    while (board.is_valid_move(shapeShadow, {0, 1})) {
        ++shapeShadow.pos.y;
    }
    return shapeShadow;
};

auto Shape::try_move(Board& board, V2 move) -> bool {
    if (board.is_valid_move(*this, move)) {
        pos.x += move.x;
        pos.y += move.y;
        return true;
    }
    return false;
}

auto Shape::get_block_positions() -> ArrayStack<Position, 4> {
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

auto Shape::get_absolute_block_positions() -> ArrayStack<Position, 4> {
    auto positions = get_block_positions();
    for (auto& position : positions) {
        position.x += pos.x;
        position.y += pos.y;
    }
    return positions;
}

auto Shape::rotate(Board& board, Rotation dir) -> bool {
    auto rotatingShape = *this;
    rotatingShape.rotationIndex += dir == Rotation::RIGHT ? 1 : -1;
    if (rotatingShape.rotationIndex == -1) rotatingShape.rotationIndex = 3;
    else if (rotatingShape.rotationIndex == 4) rotatingShape.rotationIndex = 0;
    if (board.is_valid_shape(rotatingShape)) {
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
        if (board.is_valid_shape(rotatingShape)) {
            *this = rotatingShape;
            return true;
        }
    }
    return false;
}
