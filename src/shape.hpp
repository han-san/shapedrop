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
  static constexpr u8 maxHeight {4};

  enum class RotationDirection { Left, Right };

  enum class Rotation { r0, r90, r180, r270 };

  Color::RGBA color {Color::invalid};
  Point<int> pos;

  // All shapes are composed of 4 blocks.
  static constexpr std::size_t blockCount {4};
  using BlockStack = ArrayStack<Point<int>, blockCount>;

  explicit Shape(Type type) noexcept;

  // Returns the positions of the blocks relative to the top left corner of the
  // play area
  [[nodiscard]] constexpr auto get_absolute_block_positions() const -> BlockStack {
    auto positions = get_local_block_positions();
    for (auto& localPosition : positions) {
      localPosition.x += pos.x;
      localPosition.y += pos.y;
    }
    return positions;
  }

  [[nodiscard]] constexpr auto get_wallkicks(const Shape::RotationDirection dir) const
      -> std::array<V2, 4> {
    const auto i = static_cast<gsl::index>(m_rotation);
    const auto j = static_cast<gsl::index>(dir);

    using gsl::at;

    switch (m_type) {
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

  [[nodiscard]] constexpr auto dimensions() const -> Rect<int>::Size {
    switch (m_type) {
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

  friend constexpr auto operator+=(Rotation& rotation, const RotationDirection& direction) noexcept
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

  [[nodiscard]] auto type() const noexcept -> Type { return m_type; }

  auto rotate(const RotationDirection dir) -> Shape& {
    m_rotation += dir;
    return *this;
  }

  auto translate(const V2 dir) -> Shape& {
    pos += dir;
    return *this;
  }

private:
  static constexpr Rect<std::size_t>::Size layoutDimensions {4, 4};
  using Layout = std::array<bool, layoutDimensions.w * layoutDimensions.h>;
  using RotationMap = std::array<Layout, 4>;

  // Returns the positions of the blocks relative to the top left corner of its
  // 4x4 rotation map
  [[nodiscard]] constexpr auto get_local_block_positions() const -> BlockStack {
    BlockStack positions {};
    const auto& layout = get_layout();
    for (std::size_t y {0}; y < layoutDimensions.h; ++y) {
      for (std::size_t x {0}; x < layoutDimensions.w; ++x) {
        const auto index = gsl::narrow_cast<gsl::index>(y * layoutDimensions.w + x);
        if (gsl::at(layout, index)) {
          positions.push_back({static_cast<int>(x), static_cast<int>(y)});
          if (positions.size() == positions.max_size()) {
            return positions;
          }
        }
      }
    }
    throw std::logic_error(
        fmt::format("Rotation map ({}) with rotation ({}) has fewer than 4 blocks active.",
                    static_cast<int>(m_type), static_cast<int>(m_rotation)));
  }

  [[nodiscard]] static constexpr auto to_color(const Type type) -> Color::RGBA {
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

  [[nodiscard]] constexpr auto get_layout() const -> const Layout& {
    const auto index = static_cast<RotationMap::size_type>(m_rotation);
    switch (m_type) {
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

    static constexpr std::array JLSTZ {
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

    static constexpr std::array I {
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
    static constexpr auto o = false;
    static constexpr auto X = true;

    static constexpr RotationMap I {
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
    static constexpr RotationMap L {
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
    static constexpr RotationMap J {
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
    static constexpr RotationMap O {
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
    static constexpr RotationMap S {
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
    static constexpr RotationMap Z {
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
    static constexpr RotationMap T {
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

  Type m_type;
  Rotation m_rotation {Rotation::r0};
};

class ShapePool {
public:
  static constexpr std::size_t size {7};
  using DataType = std::array<Shape::Type, size>;
  using PreviewStack = ArrayStack<Shape::Type, size * 2>;

  explicit ShapePool(const DataType& shapes);

  auto reshuffle() -> void;
  auto next_shape() -> Shape;
  [[nodiscard]] auto current_shape() const -> Shape;
  [[nodiscard]] auto get_preview_shapes_array() const -> PreviewStack;

private:
  DataType shapePool {};
  DataType previewPool {};
  DataType::size_type currentShapeIndex {0};
};
