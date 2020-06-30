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
        float textSize;
        float x;
        float y;
    };
    auto static textToDraw = std::vector<TextInfo>{};

    struct Menu {
        Squaref region;
        std::vector<Squaref> children;
    };
    auto static menus = std::vector<Menu>{};

    auto static get_current_ui_region() -> Squaref {
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

    auto static turn_relative_offset_into_window_space(Positionf const offset) -> Positionf {
        auto const workingRegion = get_current_ui_region();
        auto const x = workingRegion.x + offset.x * workingRegion.w;
        auto const y = workingRegion.y + offset.y * workingRegion.h;
        return {x, y};
    }

    auto static turn_relative_offset_into_window_space(Squaref const region) -> Squaref {
        auto const workingRegion = get_current_ui_region();
        auto const x = workingRegion.x + region.x * workingRegion.w;
        auto const y = workingRegion.y + region.y * workingRegion.h;
        auto const w = region.w * workingRegion.w;
        auto const h = region.h * workingRegion.h;
        return {x, y, w, h};
    }

    auto static add_region_as_child_of_current_menu(Squaref const region) {
        if (!menus.empty()) {
            menus.back().children.push_back(region);
        }
    }

    // TODO: The font height is currently always considered to be relative to the window space. Should it?
    auto label(std::string const text, float const fontHeight, Positionf const offset, bool const centered) -> void {
        auto const windowOffset = turn_relative_offset_into_window_space(offset);
        add_region_as_child_of_current_menu({windowOffset.x, windowOffset.y, 0, fontHeight});
        textToDraw.push_back({std::move(text), fontHeight, windowOffset.x, windowOffset.y});
    }

    // TODO: The font height is currently always considered to be relative to the window space. Should it?
    auto button(std::string const text, float const fontHeight, Positionf const offset, bool const centered) -> bool {
        auto const windowOffset = turn_relative_offset_into_window_space(offset);
        auto const w = to_normalized_width(FontString::get_text_width_normalized(text, fontHeight));
        auto const h = fontHeight;
        auto const region = Squaref{windowOffset.x, windowOffset.y, w, h};

        add_region_as_child_of_current_menu({windowOffset.x, windowOffset.y, 0, fontHeight});
        textToDraw.push_back({std::move(text), fontHeight, windowOffset.x, windowOffset.y});

        auto const screenSpaceRegion = to_screen_space(region);
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
            auto fontString = FontString::from_height_normalized(text.text, text.textSize);
            draw_font_string_normalized(bb, fontString, text.x, text.y);
        }
        textToDraw.clear();

        // FIXME: temporarily(?) reset clicked here since it should be reset before each frame
        clicked = false;
    }

    auto begin_menu(Squaref const region) -> void {
        auto const regionRelativeToWindow = turn_relative_offset_into_window_space(region);
        add_region_as_child_of_current_menu(regionRelativeToWindow);
        menus.push_back({regionRelativeToWindow, {}});
    }

    auto end_menu() -> void {
        menus.pop_back();
    }

} // namespace UI
