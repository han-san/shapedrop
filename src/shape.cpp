#include "shape.hpp"

#include "board.hpp"
#include "rangealgorithms.hpp"
#include "util.hpp"

#include <cassert>
#include <random>
#include <stdexcept>
#include <string>

using namespace std::string_literals;

Shape::Shape(Type const type) noexcept
    : m_type {type}, color {to_color(type)}, pos {Board::columns / 2 - 2, 0} {}

ShapePool::ShapePool(ShapePool::DataType const& shapes)
    : shapePool {shapes}, previewPool {shapes} {
  reshuffle();
}

auto ShapePool::reshuffle() -> void {
  // TODO: seed random engine
  shuffle(shapePool, std::default_random_engine());
  shuffle(previewPool, std::default_random_engine());
}

auto ShapePool::next_shape() -> Shape {
  ++currentShapeIndex;
  if (currentShapeIndex == shapePool.size()) {
    shapePool = previewPool;
    currentShapeIndex = 0;
    shuffle(previewPool, std::default_random_engine());
  }
  return Shape {shapePool[currentShapeIndex]};
}

auto ShapePool::current_shape() const -> Shape {
  return Shape {shapePool[currentShapeIndex]};
}

auto ShapePool::get_preview_shapes_array() const -> PreviewStack {
  PreviewStack lookaheadArray;
  for (auto i = currentShapeIndex + 1; i != shapePool.size(); ++i) {
    lookaheadArray.push_back(shapePool[i]);
  }
  for (auto const shape : previewPool) {
    lookaheadArray.push_back(shape);
  }
  return lookaheadArray;
}
