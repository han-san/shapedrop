#include <cassert>
#include <string>
#include <vector>

#include "core.hpp"
#include "draw.hpp"
#include "font.hpp"
#include "util.hpp"

#include "ui.hpp"

using namespace std::string_literals;

namespace UI {

    auto static cursor = Positionf{};
    auto static clicked = false;

    struct TextInfo {
        std::string text;
        WindowScale textSize;
        WindowScale x;
        WindowScale y;
    };
    auto static textToDraw = std::vector<TextInfo>{};

    struct Menu {
        WindowScaleRect region;
        std::vector<WindowScaleRect> children;
    };
    auto static menus = std::vector<Menu>{};

    auto static to_squaref(WindowScaleRect const rect) -> Squaref {
        return {float(rect.x), float(rect.y), float(rect.w), float(rect.h)};
    }

    auto static get_current_ui_region() -> WindowScaleRect {
        if (menus.empty()) {
            return {0.f, 0.f, 1.f, 1.f};
        } else {
            // if there is a child, the region's y coordinate should start from
            // where the child's y coordinate ends.
            auto const& menu = menus.back();
            auto currentRegion = menu.region;
            if (!menu.children.empty()) {
                auto const& child = menu.children.back();
                currentRegion.y = child.y + child.h;
                currentRegion.h = menu.region.h - (currentRegion.y - menu.region.y);
            }
            return currentRegion;
        }
    }

    auto static to_window_scale(RelativeScalePoint const offset) -> WindowScalePoint {
        auto const workingRegion = get_current_ui_region();
        auto const x = workingRegion.x + float(offset.x) * workingRegion.w;
        auto const y = workingRegion.y + float(offset.y) * workingRegion.h;
        return {x, y};
    }

    auto static to_window_scale(RelativeScaleRect const region) -> WindowScaleRect {
        auto const workingRegion = get_current_ui_region();
        auto const x = workingRegion.x + float(region.x) * workingRegion.w;
        auto const y = workingRegion.y + float(region.y) * workingRegion.h;
        auto const w = float(region.w) * workingRegion.w;
        auto const h = float(region.h) * workingRegion.h;
        return {x, y, w, h};
    }

    auto static to_window_scale(XAlignment const xAlign, RelativeScale const yOffset, WindowScale const width) -> WindowScalePoint {
        auto const workingRegion = get_current_ui_region();
        auto const y = workingRegion.y + float(yOffset) * workingRegion.h;
        auto const x = [xAlign, workingRegion, width]() {
            switch (xAlign) {
                case XAlignment::LEFT: {
                    return workingRegion.x;
                } break;
                case XAlignment::CENTER: {
                    return workingRegion.x + (workingRegion.w / 2.f) - (width / 2.f);
                } break;
                case XAlignment::RIGHT: {
                    return workingRegion.x + workingRegion.w - width;
                } break;
                default: {
                    // Gotta return something by default even though all valid
                    // enumerators are handled. Yay C++.
                    // TODO: @LOG
                    assert(0);
                    return workingRegion.x;
                }
            }
        }();

        return {x, y};
    }

    auto static add_region_as_child_of_current_menu(WindowScaleRect const region) {
        if (!menus.empty()) {
            menus.back().children.push_back(region);
        }
    }

    auto get_text_window_scale_width(std::string_view const text, WindowScale const fontHeight) -> WindowScale {
        return to_normalized_width(FontString::get_text_width_normalized(text, float(fontHeight)));
    }

    // Assumes region.w and region.h are the correct sizes for the resulting FontString.
    auto static label(std::string const text, WindowScaleRect const region) {
        add_region_as_child_of_current_menu(region);
        textToDraw.push_back({std::move(text), region.h, region.x, region.y});
    }

    auto label(std::string const text, WindowScale const fontHeight, XAlignment const xAlign, RelativeScale const yOffset) -> void {
        auto const textWidth = get_text_window_scale_width(text, fontHeight);
        auto const windowOffset = to_window_scale(xAlign, yOffset, textWidth);
        auto const region = WindowScaleRect{windowOffset.x, windowOffset.y, textWidth, fontHeight};

        label(std::move(text), region);
    }

    // TODO: The font height is currently always considered to be relative to the window space. Should it?
    auto label(std::string const text, WindowScale const fontHeight, RelativeScalePoint const offset) -> void {
        auto const windowOffset = to_window_scale(offset);
        auto const region = WindowScaleRect{windowOffset.x, windowOffset.y, 0, fontHeight};

        label(std::move(text), region);
    }

    auto static button(std::string const text, WindowScaleRect const region) -> bool {
        label(std::move(text), region);

        auto const screenSpaceRegion = to_screen_space(to_squaref(region));
        return clicked && point_is_in_rect(cursor, screenSpaceRegion);
    }

    auto button(std::string const text, WindowScale const fontHeight, XAlignment const xAlign, RelativeScale const yOffset) -> bool {
        auto const textWidth = get_text_window_scale_width(text, fontHeight);
        auto const windowOffset = to_window_scale(xAlign, yOffset, textWidth);
        auto const region = WindowScaleRect{windowOffset.x, windowOffset.y, textWidth, fontHeight};

        return button(std::move(text), region);
    }

    // TODO: The font height is currently always considered to be relative to the window space. Should it?
    auto button(std::string const text, WindowScale const fontHeight, RelativeScalePoint const offset) -> bool {
        auto const textWidth = get_text_window_scale_width(text, fontHeight);
        auto const windowOffset = to_window_scale(offset);
        auto const region = WindowScaleRect{windowOffset.x, windowOffset.y, textWidth, fontHeight};

        return button(std::move(text), region);
    }

    struct SpinBox {
        std::string text;
        size_t& value;
        size_t maxValue;
        size_t minValue;
        WindowScaleRect region;

        std::string_view static constexpr buttonsString = "<>";

        SpinBox(std::string const name, WindowScale const fontHeight, WindowScalePoint const offset, size_t& value, size_t const minValue, size_t const maxValue)
            : value{value},
            maxValue{maxValue},
            minValue{minValue}
        {
            text = std::string(buttonsString) + " "s + std::move(name) + ": "s;
            auto const textWidth = get_text_window_scale_width(text, fontHeight);
            auto const maxValueWidth = get_text_window_scale_width(std::to_string(maxValue), fontHeight);
            auto const fullTextWidth = textWidth + maxValueWidth;

            region = WindowScaleRect{offset.x, offset.y, fullTextWidth, fontHeight};
        }
    };

    // base spinbox function
    auto spinbox(SpinBox spinBox) -> void {
        auto const buttonWidth = get_text_window_scale_width(SpinBox::buttonsString, spinBox.region.h) / 2.f;
        auto const decreaseButtonRegion = WindowScaleRect{spinBox.region.x, spinBox.region.y, buttonWidth, spinBox.region.h};
        auto const increaseButtonRegion = WindowScaleRect{spinBox.region.x + buttonWidth, spinBox.region.y, buttonWidth, spinBox.region.h};
        auto const decreaseButtonScreenSpaceRegion = to_screen_space(to_squaref(decreaseButtonRegion));
        auto const increaseButtonScreenSpaceRegion = to_screen_space(to_squaref(increaseButtonRegion));

        if (clicked) {
            if (point_is_in_rect(cursor, decreaseButtonScreenSpaceRegion)) {
                if (spinBox.value > spinBox.minValue) --spinBox.value;
            } else if (point_is_in_rect(cursor, increaseButtonScreenSpaceRegion)) {
                if (spinBox.value < spinBox.maxValue) ++spinBox.value;
            }
        }

        spinBox.text += std::to_string(spinBox.value);
        label(std::move(spinBox.text), spinBox.region);
    }

    auto spinbox(std::string const text, WindowScale const fontHeight, RelativeScalePoint const offset, size_t& value, size_t const minValue, size_t const maxValue) -> void {
        auto const windowOffset = to_window_scale(offset);
        auto const spinBox = SpinBox(std::move(text), fontHeight, windowOffset, value, minValue, maxValue);
        spinbox(std::move(spinBox));
    }

    auto spinbox(std::string const text, WindowScale const fontHeight, XAlignment const xAlign, RelativeScale const yOffset, size_t& value, size_t const minValue, size_t const maxValue) -> void {
        // A SpinBox's width and height aren't dependent on the region given,
        // so the correct region can be calculated after its creation.
        auto spinBox = SpinBox(std::move(text), fontHeight, {}, value, minValue, maxValue);

        auto const windowOffset = to_window_scale(xAlign, yOffset, spinBox.region.w);
        spinBox.region.x = windowOffset.x;
        spinBox.region.y = windowOffset.y;

        spinbox(std::move(spinBox));
    }

    auto update_state(Message const message) -> void {
        if (message.type == Message::Type::MOUSEBUTTONDOWN) {
            clicked = true;
            cursor.x = float(message.x);
            cursor.y = float(message.y);
        }
    }

    auto draw(BackBuffer bb) -> void {
        assert(menus.empty());

        for (auto const& text : textToDraw) {
            draw_text_normalized(bb, std::move(text.text), float(text.x), float(text.y), float(text.textSize));
        }
        textToDraw.clear();

        // FIXME: temporarily(?) reset clicked here since it should be reset before each frame
        clicked = false;
    }

    auto begin_menu(RelativeScaleRect const region) -> void {
        auto const regionRelativeToWindow = to_window_scale(region);
        add_region_as_child_of_current_menu(regionRelativeToWindow);
        menus.push_back({regionRelativeToWindow, {}});
    }

    auto end_menu() -> void {
        menus.pop_back();
    }

} // namespace UI
