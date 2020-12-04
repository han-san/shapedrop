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
    , pos {Board::columns / 2 - 2, 0} // spawn centrally
{
    switch (ttype) {
        case Type::I: {
            color = Color::Shape::I;
            rotationMap = &IRotationMap;
        } break;
        case Type::O: {
            color = Color::Shape::O;
            rotationMap = &ORotationMap;
        } break;
        case Type::L: {
            color = Color::Shape::L;
            rotationMap = &LRotationMap;
        } break;
        case Type::J: {
            color = Color::Shape::J;
            rotationMap = &JRotationMap;
        } break;
        case Type::S: {
            color = Color::Shape::S;
            rotationMap = &SRotationMap;
        } break;
        case Type::Z: {
            color = Color::Shape::Z;
            rotationMap = &ZRotationMap;
        } break;
        case Type::T: {
            color = Color::Shape::T;
            rotationMap = &TRotationMap;
        } break;
    }
}

auto Shape::get_local_block_positions() const -> BlockStack {
    BlockStack positions {};
    auto const rotationIndex {static_cast<RotationMap::size_type>(rotation)};
    auto const& layout {(*rotationMap)[rotationIndex]};
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
    throw std::logic_error(fmt::format("The rotation map in rotationIndex ({}) has fewer than 4 blocks active.", rotationIndex));
}

auto Shape::get_absolute_block_positions() const -> BlockStack {
    auto positions {get_local_block_positions()};
    for (auto& localPosition : positions) {
        localPosition.x += pos.x;
        localPosition.y += pos.y;
    }
    return positions;
}

auto Shape::get_wallkicks(Shape::RotationDirection const dir) const -> std::array<V2, 4> {
    // Shapes J, L, S, T, and Z all have the same wall kicks while I has its
    // own and O can't kick since it doesn't rotate at all.

    std::array static constexpr JLSTZKicks {
        // r0
        std::array {
            // left
            std::array { V2 {1, 0}, V2 {1, 1}, V2 {0, -2}, V2 {1, -2} },
            // right
            std::array { V2 {-1, 0}, V2 {-1, 1}, V2 {0, -2}, V2 {-1, -2} },
        },
        // r90
        std::array {
            // both directions check the same positions
            std::array { V2 {1, 0}, V2 {1, -1}, V2 {0, 2}, V2 {1, 2} },
            std::array { V2 {1, 0}, V2 {1, -1}, V2 {0, 2}, V2 {1, 2} },
        },
        // r180
        std::array {
            std::array { V2 {-1, 0}, V2 {-1, 1}, V2 {0, -2}, V2 {-1, -2} },
            std::array { V2 {1, 0}, V2 {1, 1}, V2 {0, -2}, V2 {1, -2} },
        },
        // r270
        std::array {
            // both directions check the same positions
            std::array { V2 {-1, 0}, V2 {-1, -1}, V2 {0, 2}, V2 {-1, 2} },
            std::array { V2 {-1, 0}, V2 {-1, -1}, V2 {0, 2}, V2 {-1, 2} },
        }
    };

    std::array static constexpr IKicks {
        std::array {
            std::array { V2 {-1, 0}, V2 {2, 0}, V2 {-1, 2}, V2 {2, -1} },
            std::array { V2 {-2, 0}, V2 {1, 0}, V2 {-2, -1}, V2 {1, 2} },
        },
        std::array {
            std::array { V2 {2, 0}, V2 {-1, 0}, V2 {2, 1}, V2 {-1, -2} },
            std::array { V2 {-1, 0}, V2 {2, 0}, V2 {-1, 2}, V2 {2, -1} },
        },
        std::array {
            std::array { V2 {1, 0}, V2 {-2, 0}, V2 {1, -2}, V2 {-2, 1} },
            std::array { V2 {2, 0}, V2 {-1, 0}, V2 {2, 1}, V2 {-1, -2} },
        },
        std::array {
            std::array { V2 {-2, 0}, V2 {1, 0}, V2 {-2, -1}, V2 {1, 2} },
            std::array { V2 {1, 0}, V2 {-2, 0}, V2 {1, -2}, V2 {-2, 1} },
        },
    };

    auto const i {static_cast<std::size_t>(rotation)};
    auto const j {static_cast<std::size_t>(dir)};

    switch (type) {
        case Shape::Type::J:
        case Shape::Type::L:
        case Shape::Type::S:
        case Shape::Type::T:
        case Shape::Type::Z: {
            return JLSTZKicks[i][j];
        }
        case Shape::Type::I: {
            return IKicks[i][j];
        }
        case Shape::Type::O:
            return {};
    }
}

ShapePool::ShapePool(std::array<Shape, ShapePool::size> const& shapes)
    : shapePool {
        &shapes[0], &shapes[1], &shapes[2],
        &shapes[3], &shapes[4], &shapes[5],
        &shapes[6],
    }, previewPool {shapePool}
{
    reshuffle();
    currentShapeIterator = shapePool.begin();
}


ShapePool::ShapePool(ShapePool const& other)
{
    *this = other;
    currentShapeIterator = shapePool.begin();
}

auto ShapePool::operator=(ShapePool const& other) -> ShapePool&
{
    this->shapePool = other.shapePool;
    this->previewPool = other.previewPool;
    currentShapeIterator = shapePool.begin();
    return *this;
}

auto ShapePool::reshuffle() -> void
{
    // TODO: seed random engine
    shuffle(shapePool, std::default_random_engine());
    shuffle(previewPool, std::default_random_engine());
}

auto ShapePool::next_shape() -> Shape
{
    ++currentShapeIterator;
    if (currentShapeIterator == shapePool.end()) {
        shapePool = previewPool;
        currentShapeIterator = shapePool.begin();
        shuffle(previewPool, std::default_random_engine());
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
    for (auto it {currentShapeIterator + 1}; it != shapePool.end(); ++it) {
        lookaheadArray.push_back(*it);
    }
    for (auto const& shapePointer : previewPool) {
        lookaheadArray.push_back(shapePointer);
    }
    return lookaheadArray;
}
