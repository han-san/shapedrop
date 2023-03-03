#include "shape.hpp"

#include "board.hpp"
#include "util.hpp"

#include <cassert>
#include <random>
#include <ranges>
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
  std::ranges::shuffle(shapePool, std::default_random_engine());
  std::ranges::shuffle(previewPool, std::default_random_engine());
}

auto ShapePool::next_shape() -> Shape {
  ++currentShapeIndex;
  if (currentShapeIndex == shapePool.size()) {
    shapePool = previewPool;
    currentShapeIndex = 0;
    std::ranges::shuffle(previewPool, std::default_random_engine());
  }
  return Shape {shapePool[currentShapeIndex]};
}

auto ShapePool::current_shape() const -> Shape {
  return Shape {shapePool[currentShapeIndex]};
}

auto ShapePool::get_preview_shapes_array() const -> PreviewStack {
  using namespace std::views;
  const auto poolViews = {all(shapePool), all(previewPool)};
  auto lookaheadView = poolViews | join | drop(currentShapeIndex + 1);
  PreviewStack lookaheadArray;
  std::ranges::copy(lookaheadView, std::back_inserter(lookaheadArray));
  return lookaheadArray;
}
