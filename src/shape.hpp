#pragma once

#include "jint.h"
#include "util.hpp"

class Shape {
public:
    Rect<std::size_t>::Size static constexpr layoutDimensions {4, 4};
    using ShapeLayout = std::array<bool, layoutDimensions.w * layoutDimensions.h>;
    using RotationMap = std::array<ShapeLayout, 4>;

    enum class RotationType {
        Wallkick,
        Regular,
    };

    enum class Type {
        I, O, L, J, S, Z, T
    };

    // The shape with the maximum height is the I shape (4 blocks tall).
    u8 static constexpr maxHeight {4};

    enum class RotationDirection {
        Left,
        Right
    };

    enum class Rotation {
        r0, r90, r180, r270
    };

    auto constexpr friend operator +=(Rotation& rotationEnum, RotationDirection const& direction) -> Rotation& {
        auto constexpr minRotationValue {static_cast<int>(Rotation::r0)};
        auto constexpr maxRotationValue {static_cast<int>(Rotation::r270)};
        auto rotationInt {static_cast<int>(rotationEnum)};
        switch (direction) {
            case RotationDirection::Left: {
                --rotationInt;
                if (rotationInt < minRotationValue) {
                    rotationInt = maxRotationValue;
                }
            } break;
            case RotationDirection::Right: {
                ++rotationInt;
                if (rotationInt > maxRotationValue) {
                    rotationInt = minRotationValue;
                }
            } break;
        }
        rotationEnum = static_cast<Rotation>(rotationInt);
        return rotationEnum;
    }

    Type type;
    Rotation rotation {Rotation::r0};
    Color::RGBA color {Color::invalid};
    Point<int> pos;

    Shape(Type type) noexcept;

    // All shapes are composed of 4 blocks.
    std::size_t static constexpr blockCount {4};
    using BlockStack = ArrayStack<Point<int>, blockCount>;

    // Returns the positions of the blocks relative to the top left corner of its 4x4 rotation map
    [[nodiscard]] auto get_local_block_positions() const -> BlockStack;
    // Returns the positions of the blocks relative to the top left corner of the play area
    [[nodiscard]] auto get_absolute_block_positions() const -> BlockStack;
    [[nodiscard]] auto get_wallkicks(Shape::RotationDirection dir) const -> std::array<V2, 4>;
    [[nodiscard]] auto constexpr dimensions() const -> Rect<int>::Size {
        switch (type) {
            case Type::I:
                return {4, 1};
            case Type::O:
                return {2, 2};
            case Type::L:
            case Type::J:
            case Type::S:
            case Type::Z:
            case Type::T:
                return {3, 2};
        }
        throw; // unreachable
    }
    [[nodiscard]] auto static constexpr type_to_color(Type const type) -> Color::RGBA {
        switch (type) {
            case Type::I:
                return Color::Shape::I;
            case Type::O:
                return Color::Shape::O;
            case Type::L:
                return Color::Shape::L;
            case Type::J:
                return Color::Shape::J;
            case Type::S:
                return Color::Shape::S;
            case Type::Z:
                return Color::Shape::Z;
            case Type::T:
                return Color::Shape::T;
        }
        throw; // unreachable
    }
    [[nodiscard]] auto constexpr get_layout() const -> ShapeLayout const& {
        auto const index {static_cast<RotationMap::size_type>(rotation)};
        switch (type) {
            case Shape::Type::I:
                return IRotationMap[index];
            case Shape::Type::O:
                return ORotationMap[index];
            case Shape::Type::L:
                return LRotationMap[index];
            case Shape::Type::J:
                return JRotationMap[index];
            case Shape::Type::S:
                return SRotationMap[index];
            case Shape::Type::Z:
                return ZRotationMap[index];
            case Shape::Type::T:
                return TRotationMap[index];
        }
        throw; // unreachable
    }

private:
    RotationMap static constexpr IRotationMap {
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
    RotationMap static constexpr LRotationMap {
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
    RotationMap static constexpr JRotationMap {
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
    RotationMap static constexpr ORotationMap {
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
    RotationMap static constexpr SRotationMap {
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
    RotationMap static constexpr ZRotationMap {
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
    RotationMap static constexpr TRotationMap {
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

};

class ShapePool {
public:
    std::size_t static constexpr size {7};
private:
    using ShapePoolType = std::array<Shape const*, size>;
    using PreviewStack = ArrayStack<Shape const*, size * 2>;

    ShapePoolType shapePool;

    ShapePoolType previewPool;
    ShapePoolType::iterator currentShapeIterator;

public:
    ShapePool(std::array<Shape, size> const& shapes);
    ShapePool(ShapePool const& other);
    auto operator=(ShapePool const& other) -> ShapePool&;

    auto reshuffle() -> void;
    auto next_shape() -> Shape;
    [[nodiscard]] auto current_shape() const -> Shape;
    [[nodiscard]] auto get_preview_shapes_array() const -> PreviewStack;
};
