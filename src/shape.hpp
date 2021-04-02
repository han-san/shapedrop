#pragma once

#include "util.hpp"

#include "jint.h"

#include "fmt/core.h"

#include <exception>
#include <stdexcept>

class Shape {
public:
  enum class RotationType {
    Wallkick,
    Regular,
  };

  enum class Type { I, O, L, J, S, Z, T };

  // The shape with the maximum height is the I shape (4 blocks tall).
  u8 static constexpr maxHeight {4};

  enum class RotationDirection { Left, Right };

  enum class Rotation { r0, r90, r180, r270 };

  Type type;
  Rotation rotation {Rotation::r0};
  Color::RGBA color {Color::invalid};
  Point<int> pos;

  // All shapes are composed of 4 blocks.
  std::size_t static constexpr blockCount {4};
  using BlockStack = ArrayStack<Point<int>, blockCount>;

  explicit Shape(Type type) noexcept;

  // Returns the positions of the blocks relative to the top left corner of the
  // play area
  [[nodiscard]] auto constexpr get_absolute_block_positions() const
      -> BlockStack {
    auto positions = get_local_block_positions();
    for (auto& localPosition : positions) {
      localPosition.x += pos.x;
      localPosition.y += pos.y;
    }
    return positions;
  }

  [[nodiscard]] auto constexpr get_wallkicks(
      Shape::RotationDirection const dir) const -> std::array<V2, 4> {
    auto const i = static_cast<gsl::index>(rotation);
    auto const j = static_cast<gsl::index>(dir);

    using gsl::at;

    switch (type) {
    case Shape::Type::J:
    case Shape::Type::L:
    case Shape::Type::S:
    case Shape::Type::T:
    case Shape::Type::Z: {
      return at(at(WallKicks::JLSTZ, i), j);
    }
    case Shape::Type::I: {
      return at(at(WallKicks::I, i), j);
    }
    case Shape::Type::O:
      return {};
    }
    // Unreachable.
    std::terminate();
  }

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
    // Unreachable.
    std::terminate();
  }

  auto constexpr friend operator+=(Rotation& rotation,
                                   RotationDirection const& direction) noexcept
      -> Rotation& {
    switch (direction) {
    case RotationDirection::Left:
      switch (rotation) {
      case Rotation::r0:
        rotation = Rotation::r270;
        break;
      case Rotation::r90:
        rotation = Rotation::r0;
        break;
      case Rotation::r180:
        rotation = Rotation::r90;
        break;
      case Rotation::r270:
        rotation = Rotation::r180;
        break;
      }
      break;
    case RotationDirection::Right:
      switch (rotation) {
      case Rotation::r0:
        rotation = Rotation::r90;
        break;
      case Rotation::r90:
        rotation = Rotation::r180;
        break;
      case Rotation::r180:
        rotation = Rotation::r270;
        break;
      case Rotation::r270:
        rotation = Rotation::r0;
        break;
      }
      break;
    }

    return rotation;
  }

private:
  Rect<std::size_t>::Size static constexpr layoutDimensions {4, 4};
  using Layout = std::array<bool, layoutDimensions.w * layoutDimensions.h>;
  using RotationMap = std::array<Layout, 4>;

  // Returns the positions of the blocks relative to the top left corner of its
  // 4x4 rotation map
  [[nodiscard]] auto constexpr get_local_block_positions() const -> BlockStack {
    BlockStack positions {};
    auto const& layout = get_layout();
    for (std::size_t y {0}; y < layoutDimensions.h; ++y) {
      for (std::size_t x {0}; x < layoutDimensions.w; ++x) {
        auto const index =
            gsl::narrow_cast<gsl::index>(y * layoutDimensions.w + x);
        if (gsl::at(layout, index)) {
          positions.push_back({static_cast<int>(x), static_cast<int>(y)});
          if (positions.size() == positions.max_size()) {
            return positions;
          }
        }
      }
    }
    throw std::logic_error(fmt::format(
        "Rotation map ({}) with rotation ({}) has fewer than 4 blocks active.",
        type, rotation));
  }

  [[nodiscard]] auto static constexpr type_to_color(Type const type)
      -> Color::RGBA {
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
    // Unreachable.
    std::terminate();
  }
  [[nodiscard]] auto constexpr get_layout() const -> Layout const& {
    auto const index = static_cast<RotationMap::size_type>(rotation);
    switch (type) {
    case Shape::Type::I:
      return RotationMaps::I[index];
    case Shape::Type::O:
      return RotationMaps::O[index];
    case Shape::Type::L:
      return RotationMaps::L[index];
    case Shape::Type::J:
      return RotationMaps::J[index];
    case Shape::Type::S:
      return RotationMaps::S[index];
    case Shape::Type::Z:
      return RotationMaps::Z[index];
    case Shape::Type::T:
      return RotationMaps::T[index];
    }
    // Unreachable.
    std::terminate();
  }

  struct WallKicks {
    // Shapes J, L, S, T, and Z all have the same wall kicks while I has its
    // own and O can't kick since it doesn't rotate at all.

    std::array static constexpr JLSTZ {
        // r0
        std::array {
            // left
            std::array {V2 {1, 0}, V2 {1, 1}, V2 {0, -2}, V2 {1, -2}},
            // right
            std::array {V2 {-1, 0}, V2 {-1, 1}, V2 {0, -2}, V2 {-1, -2}},
        },
        // r90
        std::array {
            // both directions check the same positions
            std::array {V2 {1, 0}, V2 {1, -1}, V2 {0, 2}, V2 {1, 2}},
            std::array {V2 {1, 0}, V2 {1, -1}, V2 {0, 2}, V2 {1, 2}},
        },
        // r180
        std::array {
            std::array {V2 {-1, 0}, V2 {-1, 1}, V2 {0, -2}, V2 {-1, -2}},
            std::array {V2 {1, 0}, V2 {1, 1}, V2 {0, -2}, V2 {1, -2}},
        },
        // r270
        std::array {
            // both directions check the same positions
            std::array {V2 {-1, 0}, V2 {-1, -1}, V2 {0, 2}, V2 {-1, 2}},
            std::array {V2 {-1, 0}, V2 {-1, -1}, V2 {0, 2}, V2 {-1, 2}},
        }};

    std::array static constexpr I {
        std::array {
            std::array {V2 {-1, 0}, V2 {2, 0}, V2 {-1, 2}, V2 {2, -1}},
            std::array {V2 {-2, 0}, V2 {1, 0}, V2 {-2, -1}, V2 {1, 2}},
        },
        std::array {
            std::array {V2 {2, 0}, V2 {-1, 0}, V2 {2, 1}, V2 {-1, -2}},
            std::array {V2 {-1, 0}, V2 {2, 0}, V2 {-1, 2}, V2 {2, -1}},
        },
        std::array {
            std::array {V2 {1, 0}, V2 {-2, 0}, V2 {1, -2}, V2 {-2, 1}},
            std::array {V2 {2, 0}, V2 {-1, 0}, V2 {2, 1}, V2 {-1, -2}},
        },
        std::array {
            std::array {V2 {-2, 0}, V2 {1, 0}, V2 {-2, -1}, V2 {1, 2}},
            std::array {V2 {1, 0}, V2 {-2, 0}, V2 {1, -2}, V2 {-2, 1}},
        },
    };
  };

  struct RotationMaps {
    auto static constexpr o = false;
    auto static constexpr X = true;

    RotationMap static constexpr I {
        Layout {
            o, o, o, o, //
            X, X, X, X, //
            o, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, o, X, o, //
            o, o, X, o, //
            o, o, X, o, //
            o, o, X, o, //
        },
        Layout {
            o, o, o, o, //
            o, o, o, o, //
            X, X, X, X, //
            o, o, o, o, //
        },
        Layout {
            o, X, o, o, //
            o, X, o, o, //
            o, X, o, o, //
            o, X, o, o, //
        },
    };
    RotationMap static constexpr L {
        Layout {
            o, o, X, o, //
            X, X, X, o, //
            o, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, X, o, o, //
            o, X, o, o, //
            o, X, X, o, //
            o, o, o, o, //
        },
        Layout {
            o, o, o, o, //
            X, X, X, o, //
            X, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            X, X, o, o, //
            o, X, o, o, //
            o, X, o, o, //
            o, o, o, o, //
        },
    };
    RotationMap static constexpr J {
        Layout {
            X, o, o, o, //
            X, X, X, o, //
            o, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, X, X, o, //
            o, X, o, o, //
            o, X, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, o, o, o, //
            X, X, X, o, //
            o, o, X, o, //
            o, o, o, o, //
        },
        Layout {
            o, X, o, o, //
            o, X, o, o, //
            X, X, o, o, //
            o, o, o, o, //
        },
    };
    RotationMap static constexpr O {
        Layout {
            o, X, X, o, //
            o, X, X, o, //
            o, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, X, X, o, //
            o, X, X, o, //
            o, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, X, X, o, //
            o, X, X, o, //
            o, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, X, X, o, //
            o, X, X, o, //
            o, o, o, o, //
            o, o, o, o, //
        },
    };
    RotationMap static constexpr S {
        Layout {
            o, X, X, o, //
            X, X, o, o, //
            o, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, X, o, o, //
            o, X, X, o, //
            o, o, X, o, //
            o, o, o, o, //
        },
        Layout {
            o, o, o, o, //
            o, X, X, o, //
            X, X, o, o, //
            o, o, o, o, //
        },
        Layout {
            X, o, o, o, //
            X, X, o, o, //
            o, X, o, o, //
            o, o, o, o, //
        },
    };
    RotationMap static constexpr Z {
        Layout {
            X, X, o, o, //
            o, X, X, o, //
            o, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, o, X, o, //
            o, X, X, o, //
            o, X, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, o, o, o, //
            X, X, o, o, //
            o, X, X, X, //
            o, o, o, o, //
        },
        Layout {
            o, X, o, o, //
            X, X, o, o, //
            X, o, o, o, //
            o, o, o, o, //
        },
    };
    RotationMap static constexpr T {
        Layout {
            o, X, o, o, //
            X, X, X, o, //
            o, o, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, X, o, o, //
            o, X, X, o, //
            o, X, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, o, o, o, //
            X, X, X, o, //
            o, X, o, o, //
            o, o, o, o, //
        },
        Layout {
            o, X, o, o, //
            X, X, o, o, //
            o, X, o, o, //
            o, o, o, o, //
        },
    };
  };
};

class ShapePool {
public:
  std::size_t static constexpr size {7};
  using DataType = std::array<Shape::Type, size>;
  using PreviewStack = ArrayStack<Shape::Type, size * 2>;

  explicit ShapePool(DataType const& shapes);

  auto reshuffle() -> void;
  auto next_shape() -> Shape;
  [[nodiscard]] auto current_shape() const -> Shape;
  [[nodiscard]] auto get_preview_shapes_array() const -> PreviewStack;

private:
  DataType shapePool {};
  DataType previewPool {};
  DataType::size_type currentShapeIndex {0};
};
