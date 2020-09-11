#include <cassert>

#include <SDL.h>

#include "../jint.h"

#include "../core.hpp"
#include "../font.hpp"

#include "sdlmain.hpp"

namespace platform::SDL {

auto windowScale {1};

struct {
    SDL_Window* handle;
    SDL_Surface* surface;
    SDL_Surface* bbSurface;
} window {};

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

auto static resize_window(V2 const dimensions) {
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
    resize_window({gBaseWindowWidth * windowScale, gBaseWindowHeight * windowScale});
}

auto swap_buffer() -> void
{
    SDL_BlitSurface(window.bbSurface, NULL, window.surface, NULL);
    SDL_UpdateWindowSurface(window.handle);
}

auto static window_fits_on_screen(V2 windowDimensions) -> bool {
    SDL_Rect displayBounds {};
    SDL_GetDisplayUsableBounds(0, &displayBounds);

    return windowDimensions.w < displayBounds.w && windowDimensions.h < displayBounds.h;
}

auto handle_input() -> Message
{
    Message msg {};
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            msg.type = Message::Type::Quit;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_RIGHT: {
                msg.type = Message::Type::Move_right;
            } break;
            case SDLK_LEFT: {
                msg.type = Message::Type::Move_left;
            } break;
            case SDLK_r: {
                msg.type = Message::Type::Reset;
            } break;
            case SDLK_DOWN: {
                msg.type = Message::Type::Increase_speed;
            } break;
            case SDLK_UP: {
                msg.type = Message::Type::Drop;
            } break;
            case SDLK_z: {
                msg.type = Message::Type::Rotate_left;
            } break;
            case SDLK_x: {
                msg.type = Message::Type::Rotate_right;
            } break;
            case SDLK_2: {
                msg.type = Message::Type::Increase_window_size;
            } break;
            case SDLK_1: {
                msg.type = Message::Type::Decrease_window_size;
            } break;
            case SDLK_SPACE: {
                msg.type = Message::Type::Hold;
            } break;
            case SDLK_ESCAPE: {
                msg.type = Message::Type::Pause;
            } break;
            default: {
            } break;
            }
        } else if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
            case SDLK_DOWN: {
                msg.type = Message::Type::Reset_speed;
            } break;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                msg.type = Message::Type::Mousebuttondown;
                msg.x = e.button.x;
                msg.y = e.button.y;
            }
        }
    }

    return msg;
}


auto static init_window() {
    SDL_Init(SDL_INIT_EVERYTHING);

    auto newScale {windowScale};
    {
        V2 dim {};
        do {
            ++newScale;
            dim = V2 {gBaseWindowWidth * newScale, gBaseWindowHeight * newScale};
        } while (window_fits_on_screen(dim));
    }
    --newScale;
    windowScale = newScale;
    auto const initialWindowWidth {gBaseWindowWidth * windowScale};
    auto const initialWindowHeight {gBaseWindowHeight * windowScale};

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

    if (!init_font("DejaVuSans.ttf")) {
        assert(false);
        return 1;
    }

    run();

    return 0;
}
