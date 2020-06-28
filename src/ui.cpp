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
        bool centered;
    };
    auto static textToDraw = std::vector<TextInfo>{};

    struct Menu {
        std::string title;
        Squaref region;
        int children = 0;
    };
    auto static menus = std::vector<Menu>{};

    auto adjust_coords_for_menu(Positionf coords, float const fontHeight) -> Positionf {
        if (!menus.empty()) {
            // if there is an active menu, the coordinates should be based on
            // its region instead of the window's.
            auto& menu = menus.back();
            coords.x = menu.region.x + menu.region.w * coords.x;
            coords.y = menu.region.y + menu.region.h * coords.y;
            // y should be advanced based on the amount of children (+1 since the menu's label is drawn at the very top)
            coords.y += (1 + menu.children) * fontHeight;
            // FIXME: probably shouldn't increment this here since the
            // functions that want to call this might also call each other,
            // e.g. button() calling label() thereby incrementing it twice even
            // though it's really just the one button
            ++menu.children;
        }
        return coords;
    }

    auto begin_menu(std::string const title, float const fontHeight, Squaref const region) -> void {
        menus.push_back({title, region});
        textToDraw.push_back({std::move(title), fontHeight, region.x, region.y});
    }

    auto end_menu() -> void {
        menus.pop_back();
    }

    auto label(std::string const text, float const fontHeight, Positionf pos, bool const centered) -> void {
        pos = adjust_coords_for_menu(pos, fontHeight);
        textToDraw.push_back({std::move(text), fontHeight, pos.x, pos.y, centered});
    }

    auto button(std::string const text, float const fontHeight, Positionf const pos, bool const centered) -> bool {
        auto const w = to_normalized_width(FontString::get_text_width_normalized(text, fontHeight));
        auto const h = fontHeight;
        auto const coords = adjust_coords_for_menu(pos, fontHeight);
        auto const region = Squaref{coords.x, coords.y, w, h};
        auto const screenSpaceRegion = to_screen_space(region);

        textToDraw.push_back({std::move(text), fontHeight, coords.x, coords.y, centered});

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
        for (auto const& text : textToDraw) {
            auto fontString = FontString::from_height_normalized(text.text, text.textSize);
            draw_font_string_normalized(bb, fontString, text.x, text.y);
        }
        textToDraw.clear();

        // FIXME: temporarily(?) reset clicked here since it should be reset before each frame
        clicked = false;
    }

} // namespace UI
