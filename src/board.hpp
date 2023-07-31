#pragma once

#include "jint.h"
#include "shape.hpp"
#include "util.hpp"

#include <array>
#include <optional>

struct Block {
  Color::RGBA color = Color::invalid;
  bool isActive {false};
};

enum class TspinType { Regular, Mini };

class Board {
public:
  static constexpr u8 rows {22};
  static constexpr u8 columns {10};
  static constexpr u8 visibleRows {rows - 2};

  [[nodiscard]] auto block_at(gsl::index i) -> Block& { return gsl::at(m_data, i); }

  [[nodiscard]] auto block_at(gsl::index i) const -> const Block& { return gsl::at(m_data, i); }

  auto rotate_shape(Shape& shape, Shape::RotationDirection dir) const
      -> std::optional<Shape::RotationType>;
  auto try_move(Shape& shape, V2 move) const -> bool;
  [[nodiscard]] auto get_shadow(const Shape& shape) const -> Shape;
  [[nodiscard]] auto check_for_tspin(const Shape& shape, Shape::RotationType rotationType) const
      -> std::optional<TspinType>;
  [[nodiscard]] auto is_valid_spot(Point<int> pos) const -> bool;
  [[nodiscard]] auto is_valid_move(Shape shape, V2 move) const -> bool;
  [[nodiscard]] auto is_valid_shape(const Shape& shape) const -> bool;
  auto remove_full_rows() -> u8;
  auto print_board() const -> void;

private:
  [[nodiscard]] auto get_cleared_rows() const -> ArrayStack<u8, Shape::maxHeight>;

  std::array<Block, rows * columns> m_data {
      make_filled_array<Block, rows * columns>({Color::black, false})};
};
