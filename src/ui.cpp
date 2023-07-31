#include "ui.hpp"

#include "core.hpp"
#include "draw.hpp"
#include "font.hpp"
#include "util.hpp"

#include "fmt/core.h"

#include <cassert>
#include <exception>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_literals;

namespace UI {

static Point<double> cursor {};
static auto clicked = false;

struct TextInfo {
  std::string text;
  WindowScale textSize;
  Point<WindowScale> coords;
};
static std::vector<TextInfo> textToDraw {};

struct Menu {
  WindowScaleRect region;
  std::vector<WindowScaleRect> children;
};
static std::vector<Menu> menus {};

struct Background {
  WindowScaleRect region;
  Color::RGBA color;
};
static std::vector<Background> backgrounds {};

[[nodiscard]] static auto to_squaref(const WindowScaleRect rect) -> Rect<double> {
  return {double {rect.x}, double {rect.y}, double {rect.w}, double {rect.h}};
}

[[nodiscard]] static auto to_point_double(const Point<WindowScale> point) -> Point<double> {
  return {double {point.x}, double {point.y}};
}

[[nodiscard]] static auto get_current_ui_region() -> WindowScaleRect {
  if (menus.empty()) {
    return {0., 0., 1., 1.};
  }
  // if there is a child, the region's y coordinate should start from
  // where the child's y coordinate ends.
  const auto& menu = menus.back();
  auto currentRegion = menu.region;
  if (not menu.children.empty()) {
    const auto& child = menu.children.back();
    currentRegion.y = child.y + child.h;
    currentRegion.h = menu.region.h - (currentRegion.y - menu.region.y);
  }
  return currentRegion;
}

[[nodiscard]] static auto to_window_scale(const RelativeScalePoint offset) -> WindowScalePoint {
  const auto workingRegion = get_current_ui_region();
  const auto x = workingRegion.x + double {offset.x} * workingRegion.w;
  const auto y = workingRegion.y + double {offset.y} * workingRegion.h;
  return {x, y};
}

[[nodiscard]] static auto to_window_scale(const RelativeScaleRect region) -> WindowScaleRect {
  const auto workingRegion = get_current_ui_region();
  const auto x = workingRegion.x + double {region.x} * workingRegion.w;
  const auto y = workingRegion.y + double {region.y} * workingRegion.h;
  const auto w = double {region.w} * workingRegion.w;
  const auto h = double {region.h} * workingRegion.h;
  return {x, y, w, h};
}

[[nodiscard]] static auto to_window_scale(const XAlignment xAlign, const RelativeScale yOffset,
                                          const WindowScale width) -> WindowScalePoint {
  const auto workingRegion = get_current_ui_region();
  const auto y = workingRegion.y + double {yOffset} * workingRegion.h;
  const auto x = [xAlign, workingRegion, width]() {
    switch (xAlign) {
    case XAlignment::Left:
      return workingRegion.x;
    case XAlignment::Center: {
      constexpr double half {.5};
      return workingRegion.x + (workingRegion.w * half) - (width * half);
    }
    case XAlignment::Right:
      return workingRegion.x + workingRegion.w - width;
    }
    // Unreachable.
    std::terminate();
  }();

  return {x, y};
}

static auto add_region_as_child_of_current_menu(const WindowScaleRect region) {
  if (not menus.empty()) {
    menus.back().children.push_back(region);
  }
}

[[nodiscard]] static auto get_text_window_scale_width(const std::string_view text,
                                                      const WindowScale fontHeight) -> WindowScale {
  return to_normalized_width(FontString::get_text_width_normalized(text, double {fontHeight}));
}

// Assumes region.w and region.h are the correct sizes for the resulting
// FontString.
static auto label(std::string text, const WindowScaleRect region) {
  add_region_as_child_of_current_menu(region);
  textToDraw.push_back({std::move(text), region.h, {region.x, region.y}});
}

auto label(std::string text, const WindowScale fontHeight, const XAlignment xAlign,
           const RelativeScale yOffset) -> void {
  const auto textWidth = get_text_window_scale_width(text, fontHeight);
  const auto windowOffset = to_window_scale(xAlign, yOffset, textWidth);
  const auto region = WindowScaleRect {windowOffset.x, windowOffset.y, textWidth, fontHeight};

  label(std::move(text), region);
}

// TODO: The font height is currently always considered to be relative to the
// window space. Should it?
auto label(std::string text, const WindowScale fontHeight, const RelativeScalePoint offset)
    -> void {
  const auto windowOffset = to_window_scale(offset);
  const auto region = WindowScaleRect {windowOffset.x, windowOffset.y, 0., fontHeight};

  label(std::move(text), region);
}

[[nodiscard]] static auto button(std::string text, const WindowScaleRect region) -> bool {
  label(std::move(text), region);

  const auto screenSpaceRegion = to_screen_space(to_squaref(region));
  return clicked and point_is_in_rect(cursor, screenSpaceRegion);
}

auto button(std::string text, const WindowScale fontHeight, const XAlignment xAlign,
            const RelativeScale yOffset) -> bool {
  const auto textWidth = get_text_window_scale_width(text, fontHeight);
  const auto windowOffset = to_window_scale(xAlign, yOffset, textWidth);
  const WindowScaleRect region {windowOffset.x, windowOffset.y, textWidth, fontHeight};

  return button(std::move(text), region);
}

// TODO: The font height is currently always considered to be relative to the
// window space. Should it?
auto button(std::string text, const WindowScale fontHeight, const RelativeScalePoint offset)
    -> bool {
  const auto textWidth = get_text_window_scale_width(text, fontHeight);
  const auto windowOffset = to_window_scale(offset);
  const WindowScaleRect region {windowOffset.x, windowOffset.y, textWidth, fontHeight};

  return button(std::move(text), region);
}

struct SpinBox {
  int& value;
  int minValue;
  int maxValue;
  std::string text;
  WindowScaleRect region;

  static constexpr std::string_view buttonsString {"<>"};

  SpinBox(const std::string_view name, const WindowScale fontHeight, const WindowScalePoint offset,
          int& vvalue, const int mminValue, const int mmaxValue)
      : value {vvalue}, minValue {mminValue}, maxValue {mmaxValue},
        text {fmt::format("{} {}: ", buttonsString, name)} {
    const auto textWidth = get_text_window_scale_width(text, fontHeight);
    const auto maxValueWidth = get_text_window_scale_width(std::to_string(maxValue), fontHeight);
    const auto fullTextWidth = textWidth + maxValueWidth;

    region = WindowScaleRect {offset.x, offset.y, fullTextWidth, fontHeight};
  }
};

// base spinbox function
static auto spinbox(SpinBox spinBox) -> void {
  constexpr auto half = .5;
  const auto buttonWidth =
      get_text_window_scale_width(SpinBox::buttonsString, spinBox.region.h) * half;
  const WindowScaleRect decreaseButtonRegion {spinBox.region.x, spinBox.region.y, buttonWidth,
                                              spinBox.region.h};
  const WindowScaleRect increaseButtonRegion {spinBox.region.x + buttonWidth, spinBox.region.y,
                                              buttonWidth, spinBox.region.h};
  const auto decreaseButtonScreenSpaceRegion = to_screen_space(to_squaref(decreaseButtonRegion));
  const auto increaseButtonScreenSpaceRegion = to_screen_space(to_squaref(increaseButtonRegion));

  if (clicked) {
    if (point_is_in_rect(cursor, decreaseButtonScreenSpaceRegion)) {
      if (spinBox.value > spinBox.minValue) {
        --spinBox.value;
      }
    } else if (point_is_in_rect(cursor, increaseButtonScreenSpaceRegion)) {
      if (spinBox.value < spinBox.maxValue) {
        ++spinBox.value;
      }
    }
  }

  spinBox.text += std::to_string(spinBox.value);
  label(std::move(spinBox.text), spinBox.region);
}

auto spinbox(const std::string_view text, const WindowScale fontHeight,
             const RelativeScalePoint offset, int& value, const int minValue, const int maxValue)
    -> void {
  spinbox(SpinBox(text, fontHeight, to_window_scale(offset), value, minValue, maxValue));
}

auto spinbox(const std::string_view text, const WindowScale fontHeight, const XAlignment xAlign,
             const RelativeScale yOffset, int& value, const int minValue, const int maxValue)
    -> void {
  // A SpinBox's width and height aren't dependent on the region given,
  // so the correct region can be calculated after its creation.
  SpinBox spinBox(text, fontHeight, {}, value, minValue, maxValue);

  const auto windowOffset = to_window_scale(xAlign, yOffset, spinBox.region.w);
  spinBox.region.x = windowOffset.x;
  spinBox.region.y = windowOffset.y;

  spinbox(std::move(spinBox));
}

auto update_state(const Event event) -> void {
  if (event.type == Event::Type::Mousebuttondown) {
    clicked = true;
    cursor.x = static_cast<double>(event.mouseCoords.x);
    cursor.y = static_cast<double>(event.mouseCoords.y);
  }
}

auto draw(BackBuffer bb) -> void {
  assert(menus.empty());

  for (const auto& bg : backgrounds) {
    draw_solid_square_normalized(bb, to_squaref(bg.region), bg.color);
  }
  backgrounds.clear();

  for (const auto& text : textToDraw) {
    draw_text_normalized(bb, text.text, to_point_double(text.coords), double {text.textSize});
  }
  textToDraw.clear();

  // FIXME: temporarily(?) reset clicked here since it should be reset before
  // each frame
  clicked = false;
}

auto begin_menu(const RelativeScaleRect region, const Color::RGBA bgColor) -> void {
  const auto regionRelativeToWindow = to_window_scale(region);
  add_region_as_child_of_current_menu(regionRelativeToWindow);
  menus.push_back({regionRelativeToWindow, {}});

  // Don't draw fully transparent backgrounds
  if (bgColor.a) {
    backgrounds.push_back({regionRelativeToWindow, bgColor});
  }
}

auto end_menu() -> void { menus.pop_back(); }

} // namespace UI
