#include <cassert>
#include <random>
#include <stdexcept>
#include <string>

#include "fmt/core.h"

#include "board.hpp"
#include "util.hpp"
#include "rangealgorithms.hpp"

#include "shape.hpp"

using namespace std::string_literals;

Shape::Shape(Type const ttype) noexcept
    : type {ttype}
    , color {type_to_color(ttype)}
    , pos {Board::columns / 2 - 2, 0} // spawn centrally
{}

auto Shape::get_local_block_positions() const -> BlockStack {
    BlockStack positions {};
    auto const& layout {get_layout()};
    for (std::size_t y {0}; y < layoutDimensions.h; ++y) {
        for (std::size_t x {0}; x < layoutDimensions.w; ++x) {
            std::size_t const index {y * layoutDimensions.w + x};
            if (layout[index]) {
                positions.push_back({static_cast<int>(x), static_cast<int>(y)});
                if (positions.size() == positions.max_size()) {
                    return positions;
                }
            }
        }
    }
    throw std::logic_error(fmt::format("Rotation map ({}) with rotation ({}) has fewer than 4 blocks active.", type, rotation));
}

auto Shape::get_absolute_block_positions() const -> BlockStack {
    auto positions {get_local_block_positions()};
    for (auto& localPosition : positions) {
        localPosition.x += pos.x;
        localPosition.y += pos.y;
    }
    return positions;
}

ShapePool::ShapePool(ShapePool::DataType const& shapes)
    : shapePool {shapes}, previewPool {shapes}
{
    reshuffle();
}


auto ShapePool::reshuffle() -> void
{
    // TODO: seed random engine
    shuffle(shapePool, std::default_random_engine());
    shuffle(previewPool, std::default_random_engine());
}

auto ShapePool::next_shape() -> Shape
{
    ++currentShapeIndex;
    if (currentShapeIndex == shapePool.size()) {
        shapePool = previewPool;
        currentShapeIndex = 0;
        shuffle(previewPool, std::default_random_engine());
    }
    return shapePool[currentShapeIndex];
}

auto ShapePool::current_shape() const -> Shape
{
    return shapePool[currentShapeIndex];
}

auto ShapePool::get_preview_shapes_array() const -> PreviewStack
{
    PreviewStack lookaheadArray;
    for (auto i {currentShapeIndex + 1}; i != shapePool.size(); ++i) {
        lookaheadArray.push_back(shapePool[i]);
    }
    for (auto const shape : previewPool) {
        lookaheadArray.push_back(shape);
    }
    return lookaheadArray;
}
