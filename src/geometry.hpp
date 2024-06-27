#pragma once

template <typename T>
struct V2Generic {
  T x {};
  T y {};

  using ThisType = V2Generic<T>;

  [[nodiscard]] static constexpr auto right() -> ThisType { return {1, 0}; }
  [[nodiscard]] static constexpr auto left() -> ThisType { return {-1, 0}; }
  [[nodiscard]] static constexpr auto up() -> ThisType { return {0, -1}; }
  [[nodiscard]] static constexpr auto down() -> ThisType { return {0, 1}; }
};

template <typename T>
constexpr auto operator+=(V2Generic<T>& lhs, const V2Generic<T>& rhs) -> V2Generic<T>& {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}

template <typename T>
[[nodiscard]] constexpr auto operator+(V2Generic<T> lhs, const V2Generic<T>& rhs) -> V2Generic<T> {
  return lhs += rhs;
}

using V2 = V2Generic<int>;
using V2f = V2Generic<double>;

template <typename T>
struct Rect {
  T x {};
  T y {};
  T w {};
  T h {};

  struct Size {
    T w {};
    T h {};
  };
};

template <typename T>
constexpr auto operator*=(Rect<T>& lhs, const T& rhs) -> Rect<T>& {
  lhs.x *= rhs;
  lhs.y *= rhs;
  lhs.w *= rhs;
  lhs.h *= rhs;
  return lhs;
}

template <typename T>
[[nodiscard]] constexpr auto operator*(Rect<T> lhs, const T& rhs) -> Rect<T> {
  return lhs *= rhs;
}

template <typename T>
struct Point {
  T x {};
  T y {};
};

template <typename T>
constexpr auto operator+=(Point<T>& lhs, const V2Generic<T>& rhs) -> Point<T>& {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}

template <typename T>
[[nodiscard]] constexpr auto operator+(Point<T> lhs, const V2Generic<T>& rhs) -> Point<T> {
  return lhs += rhs;
}

template <typename T>
[[nodiscard]] constexpr auto point_is_in_rect(const Point<T>& point, const Rect<T>& rect) {
  return (point.x >= rect.x) and (point.x < rect.x + rect.w) and (point.y >= rect.y) and
         (point.y < rect.y + rect.h);
}
