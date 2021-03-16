#include <string_view>

#include "core.hpp"
#include "util.hpp"

#include "draw.hpp"
#include "draw_opengl.hpp"
#include "draw_software.hpp"


auto draw_solid_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color) -> void {
    switch (get_render_mode()) {
        case RenderMode::opengl: {
            OpenGLRender::draw_solid_square_normalized(sqr, color);
        } break;
        case RenderMode::software: {
            SoftwareRender::draw_solid_square_normalized(buf, sqr, color);
        } break;
    }
}
auto draw_solid_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color) -> void {
    switch (get_render_mode()) {
        case RenderMode::opengl: {
            OpenGLRender::draw_solid_square(sqr, color);
        } break;
        case RenderMode::software: {
            SoftwareRender::draw_solid_square(buf, sqr, color);
        } break;
    }
}
auto draw_hollow_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color, int borderSize) -> void {
    switch (get_render_mode()) {
        case RenderMode::opengl: {
            OpenGLRender::draw_hollow_square(buf, sqr, color, borderSize);
        } break;
        case RenderMode::software: {
            SoftwareRender::draw_hollow_square(buf, sqr, color, borderSize);
        } break;
    }
}
auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color, int borderSize) -> void {
    switch (get_render_mode()) {
        case RenderMode::opengl: {
            OpenGLRender::draw_hollow_square_normalized(buf, sqr, color, borderSize);
        } break;
        case RenderMode::software: {
            SoftwareRender::draw_hollow_square_normalized(buf, sqr, color, borderSize);
        } break;
    }
}
auto draw_font_string(BackBuffer& buf, FontString const& fontString, Point<int> coords) -> void {
    switch (get_render_mode()) {
        case RenderMode::opengl: {
            OpenGLRender::draw_font_string(buf, fontString, coords);
        } break;
        case RenderMode::software: {
            SoftwareRender::draw_font_string(buf, fontString, coords);
        } break;
    }
}
auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString, Point<double> relativeCoords) -> void {
    switch (get_render_mode()) {
        case RenderMode::opengl: {
            OpenGLRender::draw_font_string_normalized(buf, fontString, relativeCoords);
        } break;
        case RenderMode::software: {
            SoftwareRender::draw_font_string_normalized(buf, fontString, relativeCoords);
        } break;
    }
}
auto draw_text(BackBuffer& buf, std::string_view text, Point<int> coords, double pixelHeight) -> void {
    switch (get_render_mode()) {
        case RenderMode::opengl: {
            OpenGLRender::draw_text(buf, text, coords, pixelHeight);
        } break;
        case RenderMode::software: {
            SoftwareRender::draw_text(buf, text, coords, pixelHeight);
        } break;
    }
}
auto draw_text_normalized(BackBuffer& buf, std::string_view text, Point<double> relativeCoords, double pixelHeight) -> void {
    switch (get_render_mode()) {
        case RenderMode::opengl: {
            OpenGLRender::draw_text_normalized(buf, text, relativeCoords, pixelHeight);
        } break;
        case RenderMode::software: {
            SoftwareRender::draw_text_normalized(buf, text, relativeCoords, pixelHeight);
        } break;
    }
}

auto draw(ProgramState& programState, GameState& gameState) -> void {
    switch (get_render_mode()) {
        case RenderMode::opengl: {
            OpenGLRender::draw(programState, gameState);
        } break;
        case RenderMode::software: {
            SoftwareRender::draw(programState, gameState);
        } break;
    }

    swap_buffer();
}
