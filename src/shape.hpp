#pragma once

#include "jint.h"
#include "util.hpp"

class Shape {
public:
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

    Type type;
    Rotation rotation {Rotation::r0};
    Color::RGBA color {Color::invalid};
    Point<int> pos;

    // All shapes are composed of 4 blocks.
    std::size_t static constexpr blockCount {4};
    using BlockStack = ArrayStack<Point<int>, blockCount>;

    Shape(Type type) noexcept;

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

private:
    Rect<std::size_t>::Size static constexpr layoutDimensions {4, 4};
    using Layout = std::array<bool, layoutDimensions.w * layoutDimensions.h>;
    using RotationMap = std::array<Layout, 4>;

    // Returns the positions of the blocks relative to the top left corner of its 4x4 rotation map
    [[nodiscard]] auto get_local_block_positions() const -> BlockStack;
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
    [[nodiscard]] auto constexpr get_layout() const -> Layout const& {
        auto const index {static_cast<RotationMap::size_type>(rotation)};
        switch (type) {
            case Shape::Type::I:
                return RotationMaps::I[index];
            case Shape::Type::O:
                return RotationMaps::O[index];
            case Shape::Type::L:
                return RotationMaps::L[index];
            case Shape::Type::J:
                return RotationMaps::O[index];
            case Shape::Type::S:
                return RotationMaps::O[index];
            case Shape::Type::Z:
                return RotationMaps::O[index];
            case Shape::Type::T:
                return RotationMaps::O[index];
        }
        throw; // unreachable
    }

    struct RotationMaps {
        auto static constexpr o {false};
        auto static constexpr X {true};

        RotationMap static constexpr I {
            Layout {
                o, o, o, o,
                X, X, X, X,
                o, o, o, o,
                o, o, o, o,
            },
            Layout {
                o, o, X, o,
                o, o, X, o,
                o, o, X, o,
                o, o, X, o,
            },
            Layout {
                o, o, o, o,
                o, o, o, o,
                X, X, X, X,
                o, o, o, o,
            },
            Layout {
                o, X, o, o,
                o, X, o, o,
                o, X, o, o,
                o, X, o, o,
            },
        };
        RotationMap static constexpr L {
            Layout {
                o, o, X, o,
                X, X, X, o,
                o, o, o, o,
                o, o, o, o,
            },
            Layout {
                o, X, o, o,
                o, X, o, o,
                o, X, X, o,
                o, o, o, o,
            },
            Layout {
                o, o, o, o,
                X, X, X, o,
                X, o, o, o,
                o, o, o, o,
            },
            Layout {
                X, X, o, o,
                o, X, o, o,
                o, X, o, o,
                o, o, o, o,
            },
        };
        RotationMap static constexpr J {
            Layout {
                X, o, o, o,
                X, X, X, o,
                o, o, o, o,
                o, o, o, o,
            },
            Layout {
                o, X, X, o,
                o, X, o, o,
                o, X, o, o,
                o, o, o, o,
            },
            Layout {
                o, o, o, o,
                X, X, X, o,
                o, o, X, o,
                o, o, o, o,
            },
            Layout {
                o, X, o, o,
                o, X, o, o,
                X, X, o, o,
                o, o, o, o,
            },
        };
        RotationMap static constexpr O {
            Layout {
                o, X, X, o,
                o, X, X, o,
                o, o, o, o,
                o, o, o, o,
            },
            Layout {
                o, X, X, o,
                o, X, X, o,
                o, o, o, o,
                o, o, o, o,
            },
            Layout {
                o, X, X, o,
                o, X, X, o,
                o, o, o, o,
                o, o, o, o,
            },
            Layout {
                o, X, X, o,
                o, X, X, o,
                o, o, o, o,
                o, o, o, o,
            },
        };
        RotationMap static constexpr S {
            Layout {
                o, X, X, o,
                X, X, o, o,
                o, o, o, o,
                o, o, o, o,
            },
            Layout {
                o, X, o, o,
                o, X, X, o,
                o, o, X, o,
                o, o, o, o,
            },
            Layout {
                o, o, o, o,
                o, X, X, o,
                X, X, o, o,
                o, o, o, o,
            },
            Layout {
                X, o, o, o,
                X, X, o, o,
                o, X, o, o,
                o, o, o, o,
            },
        };
        RotationMap static constexpr Z {
            Layout {
                X, X, o, o,
                o, X, X, o,
                o, o, o, o,
                o, o, o, o,
            },
            Layout {
                o, o, X, o,
                o, X, X, o,
                o, X, o, o,
                o, o, o, o,
            },
            Layout {
                o, o, o, o,
                X, X, o, o,
                o, X, X, X,
                o, o, o, o,
            },
            Layout {
                o, X, o, o,
                X, X, o, o,
                X, o, o, o,
                o, o, o, o,
            },
        };
        RotationMap static constexpr T {
            Layout {
                o, X, o, o,
                X, X, X, o,
                o, o, o, o,
                o, o, o, o,
            },
            Layout {
                o, X, o, o,
                o, X, X, o,
                o, X, o, o,
                o, o, o, o,
            },
            Layout {
                o, o, o, o,
                X, X, X, o,
                o, X, o, o,
                o, o, o, o,
            },
            Layout {
                o, X, o, o,
                X, X, o, o,
                o, X, o, o,
                o, o, o, o,
            },
        };
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
