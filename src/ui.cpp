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

Point<double> static cursor {};
auto static clicked = false;

struct TextInfo {
  std::string text;
  WindowScale textSize;
  Point<WindowScale> coords;
};
std::vector<TextInfo> static textToDraw {};

struct Menu {
  WindowScaleRect region;
  std::vector<WindowScaleRect> children;
};
std::vector<Menu> static menus {};

struct Background {
  WindowScaleRect region;
  Color::RGBA color;
};
std::vector<Background> static backgrounds {};

[[nodiscard]] auto static to_squaref(WindowScaleRect const rect)
    -> Rect<double> {
  return {double {rect.x}, double {rect.y}, double {rect.w}, double {rect.h}};
}

[[nodiscard]] auto static to_point_double(Point<WindowScale> const point)
    -> Point<double> {
  return {double {point.x}, double {point.y}};
}

[[nodiscard]] auto static get_current_ui_region() -> WindowScaleRect {
  if (menus.empty()) {
    return {0., 0., 1., 1.};
  }
  // if there is a child, the region's y coordinate should start from
  // where the child's y coordinate ends.
  auto const& menu = menus.back();
  auto currentRegion = menu.region;
  if (not menu.children.empty()) {
    auto const& child = menu.children.back();
    currentRegion.y = child.y + child.h;
    currentRegion.h = menu.region.h - (currentRegion.y - menu.region.y);
  }
  return currentRegion;
}

[[nodiscard]] auto static to_window_scale(RelativeScalePoint const offset)
    -> WindowScalePoint {
  auto const workingRegion = get_current_ui_region();
  auto const x = workingRegion.x + double {offset.x} * workingRegion.w;
  auto const y = workingRegion.y + double {offset.y} * workingRegion.h;
  return {x, y};
}

[[nodiscard]] auto static to_window_scale(RelativeScaleRect const region)
    -> WindowScaleRect {
  auto const workingRegion = get_current_ui_region();
  auto const x = workingRegion.x + double {region.x} * workingRegion.w;
  auto const y = workingRegion.y + double {region.y} * workingRegion.h;
  auto const w = double {region.w} * workingRegion.w;
  auto const h = double {region.h} * workingRegion.h;
  return {x, y, w, h};
}

[[nodiscard]] auto static to_window_scale(XAlignment const xAlign,
                                          RelativeScale const yOffset,
                                          WindowScale const width)
    -> WindowScalePoint {
  auto const workingRegion = get_current_ui_region();
  auto const y = workingRegion.y + double {yOffset} * workingRegion.h;
  auto const x = [xAlign, workingRegion, width]() {
    switch (xAlign) {
    case XAlignment::Left:
      return workingRegion.x;
    case XAlignment::Center: {
      double constexpr half {.5};
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

auto static add_region_as_child_of_current_menu(WindowScaleRect const region) {
  if (not menus.empty()) {
    menus.back().children.push_back(region);
  }
}

[[nodiscard]] auto static get_text_window_scale_width(
    std::string_view const text, WindowScale const fontHeight) -> WindowScale {
  return to_normalized_width(
      FontString::get_text_width_normalized(text, double {fontHeight}));
}

// Assumes region.w and region.h are the correct sizes for the resulting
// FontString.
auto static label(std::string text, WindowScaleRect const region) {
  add_region_as_child_of_current_menu(region);
  textToDraw.push_back({std::move(text), region.h, {region.x, region.y}});
}

auto label(std::string text, WindowScale const fontHeight,
           XAlignment const xAlign, RelativeScale const yOffset) -> void {
  auto const textWidth = get_text_window_scale_width(text, fontHeight);
  auto const windowOffset = to_window_scale(xAlign, yOffset, textWidth);
  auto const region =
      WindowScaleRect {windowOffset.x, windowOffset.y, textWidth, fontHeight};

  label(std::move(text), region);
}

// TODO: The font height is currently always considered to be relative to the
// window space. Should it?
auto label(std::string text, WindowScale const fontHeight,
           RelativeScalePoint const offset) -> void {
  auto const windowOffset = to_window_scale(offset);
  auto const region =
      WindowScaleRect {windowOffset.x, windowOffset.y, 0., fontHeight};

  label(std::move(text), region);
}

[[nodiscard]] auto static button(std::string text, WindowScaleRect const region)
    -> bool {
  label(std::move(text), region);

  auto const screenSpaceRegion = to_screen_space(to_squaref(region));
  return clicked && point_is_in_rect(cursor, screenSpaceRegion);
}

auto button(std::string text, WindowScale const fontHeight,
            XAlignment const xAlign, RelativeScale const yOffset) -> bool {
  auto const textWidth = get_text_window_scale_width(text, fontHeight);
  auto const windowOffset = to_window_scale(xAlign, yOffset, textWidth);
  WindowScaleRect const region {windowOffset.x, windowOffset.y, textWidth,
                                fontHeight};

  return button(std::move(text), region);
}

// TODO: The font height is currently always considered to be relative to the
// window space. Should it?
auto button(std::string text, WindowScale const fontHeight,
            RelativeScalePoint const offset) -> bool {
  auto const textWidth = get_text_window_scale_width(text, fontHeight);
  auto const windowOffset = to_window_scale(offset);
  WindowScaleRect const region {windowOffset.x, windowOffset.y, textWidth,
                                fontHeight};

  return button(std::move(text), region);
}

struct SpinBox {
  int& value;
  int minValue;
  int maxValue;
  std::string text;
  WindowScaleRect region;

  std::string_view static constexpr buttonsString {"<>"};

  SpinBox(std::string_view const name, WindowScale const fontHeight,
          WindowScalePoint const offset, int& vvalue, int const mminValue,
          int const mmaxValue)
      : value {vvalue}, minValue {mminValue}, maxValue {mmaxValue},
        text {fmt::format("{} {}: ", buttonsString, name)} {
    auto const textWidth = get_text_window_scale_width(text, fontHeight);
    auto const maxValueWidth =
        get_text_window_scale_width(std::to_string(maxValue), fontHeight);
    auto const fullTextWidth = textWidth + maxValueWidth;

    region = WindowScaleRect {offset.x, offset.y, fullTextWidth, fontHeight};
  }
};

// base spinbox function
auto static spinbox(SpinBox spinBox) -> void {
  auto constexpr half = .5;
  auto const buttonWidth =
      get_text_window_scale_width(SpinBox::buttonsString, spinBox.region.h) *
      half;
  WindowScaleRect const decreaseButtonRegion {
      spinBox.region.x, spinBox.region.y, buttonWidth, spinBox.region.h};
  WindowScaleRect const increaseButtonRegion {spinBox.region.x + buttonWidth,
                                              spinBox.region.y, buttonWidth,
                                              spinBox.region.h};
  auto const decreaseButtonScreenSpaceRegion =
      to_screen_space(to_squaref(decreaseButtonRegion));
  auto const increaseButtonScreenSpaceRegion =
      to_screen_space(to_squaref(increaseButtonRegion));

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

auto spinbox(std::string_view const text, WindowScale const fontHeight,
             RelativeScalePoint const offset, int& value, int const minValue,
             int const maxValue) -> void {
  spinbox(SpinBox(text, fontHeight, to_window_scale(offset), value, minValue,
                  maxValue));
}

auto spinbox(std::string_view const text, WindowScale const fontHeight,
             XAlignment const xAlign, RelativeScale const yOffset, int& value,
             int const minValue, int const maxValue) -> void {
  // A SpinBox's width and height aren't dependent on the region given,
  // so the correct region can be calculated after its creation.
  SpinBox spinBox(text, fontHeight, {}, value, minValue, maxValue);

  auto const windowOffset = to_window_scale(xAlign, yOffset, spinBox.region.w);
  spinBox.region.x = windowOffset.x;
  spinBox.region.y = windowOffset.y;

  spinbox(std::move(spinBox));
}

auto update_state(Event const event) -> void {
  if (event.type == Event::Type::Mousebuttondown) {
    clicked = true;
    cursor.x = static_cast<double>(event.mouseCoords.x);
    cursor.y = static_cast<double>(event.mouseCoords.y);
  }
}

auto draw(BackBuffer bb) -> void {
  assert(menus.empty());

  for (auto const& bg : backgrounds) {
    draw_solid_square_normalized(bb, to_squaref(bg.region), bg.color);
  }
  backgrounds.clear();

  for (auto const& text : textToDraw) {
    draw_text_normalized(bb, text.text, to_point_double(text.coords),
                         double {text.textSize});
  }
  textToDraw.clear();

  // FIXME: temporarily(?) reset clicked here since it should be reset before
  // each frame
  clicked = false;
}

auto begin_menu(RelativeScaleRect const region, Color::RGBA const bgColor)
    -> void {
  auto const regionRelativeToWindow = to_window_scale(region);
  add_region_as_child_of_current_menu(regionRelativeToWindow);
  menus.push_back({regionRelativeToWindow, {}});

  // Don't draw fully transparent backgrounds
  if (bgColor.a) {
    backgrounds.push_back({regionRelativeToWindow, bgColor});
  }
}

auto end_menu() -> void { menus.pop_back(); }

} // namespace UI
