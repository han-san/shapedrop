#include <cassert>
#include <string>
#include <vector>

#include "core.hpp"
#include "draw.hpp"
#include "font.hpp"
#include "util.hpp"

#include "ui.hpp"

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

    auto static add_region_as_child_of_current_menu(WindowScaleRect const region) {
        if (!menus.empty()) {
            menus.back().children.push_back(region);
        }
    }

    // TODO: The font height is currently always considered to be relative to the window space. Should it?
    auto label(std::string const text, WindowScale const fontHeight, RelativeScalePoint const offset, bool const centered) -> void {
        auto const windowOffset = to_window_scale(offset);
        add_region_as_child_of_current_menu({windowOffset.x, windowOffset.y, 0, fontHeight});
        textToDraw.push_back({std::move(text), fontHeight, windowOffset.x, windowOffset.y});
    }

    // TODO: The font height is currently always considered to be relative to the window space. Should it?
    auto button(std::string const text, WindowScale const fontHeight, RelativeScalePoint const offset, bool const centered) -> bool {
        auto const windowOffset = to_window_scale(offset);
        auto const w = to_normalized_width(FontString::get_text_width_normalized(text, float(fontHeight)));
        auto const h = fontHeight;
        auto const region = WindowScaleRect{windowOffset.x, windowOffset.y, w, h};

        add_region_as_child_of_current_menu(region);
        textToDraw.push_back({std::move(text), region.h, region.x, region.y});

        auto const screenSpaceRegion = to_screen_space({float(region.x), float(region.y), float(w), float(h)});
        return clicked && point_is_in_rect(cursor, screenSpaceRegion);
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
            auto fontString = FontString::from_height_normalized(text.text, float(text.textSize));
            draw_font_string_normalized(bb, fontString, float(text.x), float(text.y));
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
