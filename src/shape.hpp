#pragma once

#include "util.hpp"

class Shape {
public:
    std::size_t static constexpr layoutW {4};
    std::size_t static constexpr layoutH {4};
    using ShapeLayout = std::array<bool, layoutW * layoutH>;
    using RotationMap = std::array<ShapeLayout, 4>;

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

    enum class RotationType {
        Wallkick,
        Regular,
    };

    // ShapeDims relies on the specific order of these.
    // Don't move them around!
    enum class Type {
        I, O, L, J, S, Z, T
    };

    std::array<V2, 7> static constexpr dimensions { V2 {4, 1}, {2, 2}, {3, 2}, {3, 2}, {3, 2}, {3, 2}, {3, 2} };

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
    RotationMap const* rotationMap {nullptr};
    Rotation rotation {Rotation::r0};
    Color::RGBA color;
    Position pos;

    Shape(Type type) noexcept;

    // All shapes are composed of 4 blocks.
    std::size_t static constexpr BLOCK_COUNT {4};
    using BlockStack = ArrayStack<Position, BLOCK_COUNT>;

    auto get_block_positions() const -> BlockStack;
    auto get_absolute_block_positions() const -> BlockStack;
    auto get_wallkicks(Shape::RotationDirection dir) const -> std::array<V2, 4>;
};

class ShapePool {
public:
    std::size_t static constexpr SIZE {7};
private:
    using ShapePoolType = std::array<Shape const*, SIZE>;
    using PreviewStack = ArrayStack<Shape const*, SIZE * 2>;

    ShapePoolType shapePool;

    ShapePoolType previewPool;
    ShapePoolType::iterator currentShapeIterator;

public:
    ShapePool(std::array<Shape, SIZE> const& shapes);
    ShapePool(ShapePool const& other);
    ShapePool& operator=(ShapePool const& other);

    auto reshuffle() -> void;
    auto next_shape() -> Shape;
    auto current_shape() const -> Shape;
    auto get_preview_shapes_array() const -> PreviewStack;
};
