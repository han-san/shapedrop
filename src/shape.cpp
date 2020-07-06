#include <algorithm>
#include <cassert>
#include <random>

#include "board.hpp"

#include "shape.hpp"

Shape::Shape(Type const type)
    : pos{Board::columns / 2 - 2, 0} // spawn centrally
    , type{type}
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

auto Shape::get_block_positions() -> BlockStack {
    BlockStack positions = {};
    auto const& layout = (*rotations)[rotationIndex];
    for (auto y = 0; y < layoutH; ++y) {
        for (auto x = 0; x < layoutW; ++x) {
            auto const index = y * layoutW + x;
            if (layout[index]) {
                positions.push_back({x, y});
                if (positions.size() == positions.max_size()) {
                    return positions;
                }
            }
        }
    }
    assert(false);
}

auto Shape::get_absolute_block_positions() -> BlockStack {
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

auto ShapePool::current_shape() -> Shape
{
    return **currentShapeIterator;
}

auto ShapePool::get_preview_shapes_array() -> ArrayStack<Shape const*, ShapePool::SIZE * 2>
{
    ArrayStack<Shape const*, ShapePool::SIZE * 2> lookaheadArray = {};
    for (auto it = currentShapeIterator + 1; it != shapePool.end(); ++it) {
        lookaheadArray.push_back(*it);
    }
    for (auto it = previewPool.cbegin(); it != previewPool.cend(); ++it) {
        lookaheadArray.push_back(*it);
    }
    return lookaheadArray;
}
