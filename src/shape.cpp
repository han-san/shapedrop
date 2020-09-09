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
            rotations = &IRotationMap;
        } break;
        case Type::O: {
            color = RGB(0xf0, 0xf0, 0x00);
            rotations = &ORotationMap;
        } break;
        case Type::L: {
            color = RGB(0xf0, 0xa0, 0x00);
            rotations = &LRotationMap;
        } break;
        case Type::J: {
            color = RGB(0x00, 0x00, 0xf0);
            rotations = &JRotationMap;
        } break;
        case Type::S: {
            color = RGB(0x00, 0xf0, 0x00);
            rotations = &SRotationMap;
        } break;
        case Type::Z: {
            color = RGB(0xf0, 0x00, 0x00);
            rotations = &ZRotationMap;
        } break;
        case Type::T: {
            color = RGB(0xa0, 0x00, 0xf0);
            rotations = &TRotationMap;
        } break;
        default: {
            // shouldn't be possible
            assert(false);
        } break;
    }
}

auto Shape::get_block_positions() const -> BlockStack {
    BlockStack positions = {};
    auto const& layout = (*rotations)[rotationIndex];
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
