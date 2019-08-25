#include <cassert>
#include <string_view>
#include <cstdio>

#include <SDL2/SDL.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "sdlmain.hpp"
#include "core.hpp"

#include "jint.h"

namespace platform::SDL {

auto windowScale = 1;

struct {
    SDL_Window* handle;
    SDL_Surface* surface;
    SDL_Surface* bbSurface;
} window = {};

uchar static ttf_buffer[1<<25];
stbtt_fontinfo font;

auto init_font(std::string_view filePath) -> bool
{
    auto file = fopen(filePath.data(), "rb");
    if (!file) return false;
    fread(ttf_buffer, 1, 1<<25, file);
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(&(*ttf_buffer), 0));
    return true;
}

FontCharacter::FontCharacter(char c)
    : scale(stbtt_ScaleForPixelHeight(&font, 48))
{
    bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, &xoff, &yoff);
    stbtt_GetFontVMetrics(&font, &ascent, 0, 0);
}

FontCharacter::~FontCharacter()
{
    stbtt_FreeBitmap(bitmap, font.userdata); // TODO: find out this actually does
}

auto get_codepoint_kern_advance(char codepoint, char nextCodepoint, float scale) -> float
{
    int advance;
    int lsb;
    stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &lsb);
    return scale * (advance + stbtt_GetCodepointKernAdvance(&font, codepoint, nextCodepoint));
}

auto get_window_scale() -> int
{
    return windowScale;
}

auto get_back_buffer() -> BackBuffer
{
    auto bbuf = BackBuffer{};
    bbuf.memory = window.bbSurface->pixels;
    bbuf.w = window.bbSurface->w;
    bbuf.h = window.bbSurface->h;
    bbuf.pitch = window.bbSurface->pitch;
    bbuf.bpp = window.bbSurface->format->BytesPerPixel;

    return bbuf;
}

auto get_window_dimensions() -> V2
{
    return {window.surface->w, window.surface->h};
}

auto resize_window(V2 dimensions) {
    SDL_SetWindowSize(window.handle, dimensions.w, dimensions.h);
    window.surface = SDL_GetWindowSurface(window.handle);
    assert(window.surface);
    SDL_FreeSurface(window.bbSurface);
    window.bbSurface = SDL_CreateRGBSurface(0, window.surface->w, window.surface->h,
                                      window.surface->format->BitsPerPixel,
                                      window.surface->format->Rmask,
                                      window.surface->format->Gmask,
                                      window.surface->format->Bmask,
                                      window.surface->format->Amask);
    assert(window.bbSurface);
}

auto change_window_scale(int newScale) -> void {
    if (newScale < 1) newScale = 1;
    if (windowScale == newScale) return;
    windowScale = newScale;
    resize_window({baseWindowWidth * windowScale, baseWindowHeight * windowScale});
}

auto swap_buffer() -> void
{
    SDL_BlitSurface(window.bbSurface, NULL, window.surface, NULL);
    SDL_UpdateWindowSurface(window.handle);
}

auto window_fits_on_screen(V2 windowDimensions) -> bool {
    SDL_Rect displayBounds = {};
    SDL_GetDisplayUsableBounds(0, &displayBounds);

    return windowDimensions.w < displayBounds.w && windowDimensions.h < displayBounds.h;
}

auto handle_input() -> Message
{
    auto msg = Message {};
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            msg = Message::QUIT;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_RIGHT: {
                msg = Message::MOVE_RIGHT;
            } break;
            case SDLK_LEFT: {
                msg = Message::MOVE_LEFT;
            } break;
            case SDLK_r: {
                msg = Message::RESET;
            } break;
            case SDLK_DOWN: {
                msg = Message::INCREASE_SPEED;
            } break;
            case SDLK_UP: {
                msg = Message::DROP;
            } break;
            case SDLK_z: {
                msg = Message::ROTATE_LEFT;
            } break;
            case SDLK_x: {
                msg = Message::ROTATE_RIGHT;
            } break;
            case SDLK_2: {
                msg = Message::INCREASE_WINDOW_SIZE;
            } break;
            case SDLK_1: {
                msg = Message::DECREASE_WINDOW_SIZE;
            } break;
            default: {
            } break;
            }
        } else if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
            case SDLK_DOWN: {
                msg = Message::RESET_SPEED;
            } break;
            }
        }
    }

    return msg;
}


auto init_window() {
    SDL_Init(SDL_INIT_EVERYTHING);

    auto newScale = windowScale;
    {
        auto dim = V2 {};
        do {
            ++newScale;
            dim = V2 {baseWindowWidth * newScale, baseWindowHeight * newScale};
        } while (window_fits_on_screen(dim));
    }
    --newScale;
    windowScale = newScale;
    auto initialWindowWidth = baseWindowWidth * windowScale;
    auto initialWindowHeight = baseWindowHeight * windowScale;

    window.handle = SDL_CreateWindow("Tetris",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               initialWindowWidth, initialWindowHeight, SDL_WINDOW_SHOWN);
    assert(window.handle);

    window.surface = SDL_GetWindowSurface(window.handle);
    assert(window.surface);

    window.bbSurface = SDL_CreateRGBSurface(0, window.surface->w, window.surface->h,
                                      window.surface->format->BitsPerPixel,
                                      window.surface->format->Rmask,
                                      window.surface->format->Gmask,
                                      window.surface->format->Bmask,
                                      window.surface->format->Amask);
    assert(window.bbSurface);
}

} // namespace SDL

auto main(int argc, char** argv) -> int {
    if (argc || argv) {}

    platform::SDL::init_window();

    if (!platform::SDL::init_font("c:/windows/fonts/arialbd.ttf")) {
        assert(false);
        return 1;
    }

    run();

    return 0;
}
