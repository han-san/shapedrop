#include <algorithm>
#include <cassert>
#include <random>
#include <stdexcept>
#include <string>

#include "fmt/core.h"

#include "board.hpp"

#include "shape.hpp"

using namespace std::string_literals;

Shape::Shape(Type const type) noexcept
    : type{type}
    , pos{Board::columns / 2 - 2, 0} // spawn centrally
{
    switch (type) {
        case Type::I: {
            color = RGB(0x00, 0xf0, 0xf0);
            rotationMap = &IRotationMap;
        } break;
        case Type::O: {
            color = RGB(0xf0, 0xf0, 0x00);
            rotationMap = &ORotationMap;
        } break;
        case Type::L: {
            color = RGB(0xf0, 0xa0, 0x00);
            rotationMap = &LRotationMap;
        } break;
        case Type::J: {
            color = RGB(0x00, 0x00, 0xf0);
            rotationMap = &JRotationMap;
        } break;
        case Type::S: {
            color = RGB(0x00, 0xf0, 0x00);
            rotationMap = &SRotationMap;
        } break;
        case Type::Z: {
            color = RGB(0xf0, 0x00, 0x00);
            rotationMap = &ZRotationMap;
        } break;
        case Type::T: {
            color = RGB(0xa0, 0x00, 0xf0);
            rotationMap = &TRotationMap;
        } break;
    }
}

auto Shape::get_block_positions() const -> BlockStack {
    BlockStack positions = {};
    auto const rotationIndex = static_cast<RotationMap::size_type>(rotation);
    auto const& layout = (*rotationMap)[rotationIndex];
    for (size_t y = 0; y < layoutH; ++y) {
        for (size_t x = 0; x < layoutW; ++x) {
            auto const index = y * layoutW + x;
            if (layout[index]) {
                positions.push_back({int(x), int(y)});
                if (positions.size() == positions.max_size()) {
                    return positions;
                }
            }
        }
    }
    throw std::logic_error(fmt::format("The rotation map in rotationIndex ({}) has fewer than 4 blocks active.", rotationIndex));
}

auto Shape::get_absolute_block_positions() const -> BlockStack {
    auto positions = get_block_positions();
    for (auto& position : positions) {
        position.x += pos.x;
        position.y += pos.y;
    }
    return positions;
}

auto Shape::get_wallkicks(Shape::RotationDirection const dir) const -> std::array<V2, 4> {
    // Shapes J, L, S, T, and Z all have the same wall kicks while I has its
    // own and O can't kick since it doesn't rotate at all.
    switch (type) {
        case Shape::Type::J:
        case Shape::Type::L:
        case Shape::Type::S:
        case Shape::Type::T:
        case Shape::Type::Z: {
            switch (rotation) {
                case Shape::Rotation::r0: {
                    if (dir == Shape::RotationDirection::RIGHT) {
                        return { V2 {-1, 0}, {-1, 1}, {0, -2}, {-1, -2} };
                    } else {
                        return { V2 {1, 0}, {1, 1}, {0, -2}, {1, -2} };
                    }
                }
                case Shape::Rotation::r90:
                    // both directions check same positions
                    return { V2 {1, 0}, {1, -1}, {0, 2}, {1, 2} };
                case Shape::Rotation::r180: {
                    if (dir == Shape::RotationDirection::RIGHT) {
                        return { V2 {1, 0}, {1, 1}, {0, -2}, {1, -2} };
                    } else {
                        return { V2 {-1, 0}, {-1, 1}, {0, -2}, {-1, -2} };
                    }
                }
                case Shape::Rotation::r270:
                    // both directions check same positions
                    return { V2 {-1, 0}, {-1, -1}, {0, 2}, {-1, 2} };
            }
        } break;
        case Shape::Type::I: {
            switch (rotation) {
                case Shape::Rotation::r0: {
                    if (dir == Shape::RotationDirection::RIGHT) {
                        return { V2 {-2, 0}, {1, 0}, {-2, -1}, {1, 2} };
                    } else {
                        return { V2 {-1, 0}, {2, 0}, {-1, 2}, {2, -1} };
                    }
                }
                case Shape::Rotation::r90: {
                    if (dir == Shape::RotationDirection::RIGHT) {
                        return { V2 {-1, 0}, {2, 0}, {-1, 2}, {2, -1} };
                    } else {
                        return { V2 {2, 0}, {-1, 0}, {2, 1}, {-1, -2} };
                    }
                }
                case Shape::Rotation::r180: {
                    if (dir == Shape::RotationDirection::RIGHT) {
                        return { V2 {2, 0}, {-1, 0}, {2, 1}, {-1, -2} };
                    } else {
                        return { V2 {1, 0}, {-2, 0}, {1, -2}, {-2, 1} };
                    }
                }
                case Shape::Rotation::r270: {
                    if (dir == Shape::RotationDirection::RIGHT) {
                        return { V2 {1, 0}, {-2, 0}, {1, -2}, {-2, 1} };
                    } else {
                        return { V2 {-2, 0}, {1, 0}, {-2, -1}, {1, 2} };
                    }
                }
            }
        } break;
        case Shape::Type::O:
            return {};
    }
}

ShapePool::ShapePool(std::array<Shape, ShapePool::SIZE> const& shapes)
{
    shapePool = {
        &shapes[0], &shapes[1], &shapes[2],
        &shapes[3], &shapes[4], &shapes[5],
        &shapes[6],
    };
    previewPool = shapePool;

    reshuffle();
    currentShapeIterator = shapePool.begin();
}


ShapePool::ShapePool(ShapePool const& other)
{
    *this = other;
    currentShapeIterator = shapePool.begin();
}

ShapePool& ShapePool::operator=(ShapePool const& other)
{
    this->shapePool = other.shapePool;
    this->previewPool = other.previewPool;
    currentShapeIterator = shapePool.begin();
    return *this;
}

auto ShapePool::reshuffle() -> void
{
    // TODO: seed random engine
    std::shuffle(shapePool.begin(), shapePool.end(), std::default_random_engine());
    std::shuffle(previewPool.begin(), previewPool.end(), std::default_random_engine());
}

auto ShapePool::next_shape() -> Shape
{
    ++currentShapeIterator;
    if (currentShapeIterator == shapePool.end()) {
        shapePool = previewPool;
        currentShapeIterator = shapePool.begin();
        std::shuffle(previewPool.begin(), previewPool.end(), std::default_random_engine());
    }
    return **currentShapeIterator;
}

auto ShapePool::current_shape() const -> Shape
{
    return **currentShapeIterator;
}

auto ShapePool::get_preview_shapes_array() const -> PreviewStack
{
    PreviewStack lookaheadArray;
    for (auto it = currentShapeIterator + 1; it != shapePool.end(); ++it) {
        lookaheadArray.push_back(*it);
    }
    for (auto const& shapePointer : previewPool) {
        lookaheadArray.push_back(shapePointer);
    }
    return lookaheadArray;
}
