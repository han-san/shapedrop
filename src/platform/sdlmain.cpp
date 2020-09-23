#include <cassert>

#include <SDL.h>

#include "../jint.h"

#include "../core.hpp"
#include "../input.hpp"
#include "../font.hpp"
#include "../util.hpp"

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

auto get_window_dimensions() -> Rect<int>::Size
{
    return {window.surface->w, window.surface->h};
}

auto static resize_window(Rect<int>::Size const dimensions) {
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

auto static window_fits_on_screen(Rect<int>::Size windowDimensions) -> bool {
    SDL_Rect displayBounds {};
    SDL_GetDisplayUsableBounds(0, &displayBounds);

    return windowDimensions.w < displayBounds.w && windowDimensions.h < displayBounds.h;
}

auto get_event() -> Event
{
    Event event {};
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            event.type = Event::Type::Quit;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_RIGHT: {
                event.type = Event::Type::Move_right;
            } break;
            case SDLK_LEFT: {
                event.type = Event::Type::Move_left;
            } break;
            case SDLK_r: {
                event.type = Event::Type::Reset;
            } break;
            case SDLK_DOWN: {
                event.type = Event::Type::Increase_speed;
            } break;
            case SDLK_UP: {
                event.type = Event::Type::Drop;
            } break;
            case SDLK_z: {
                event.type = Event::Type::Rotate_left;
            } break;
            case SDLK_x: {
                event.type = Event::Type::Rotate_right;
            } break;
            case SDLK_2: {
                event.type = Event::Type::Increase_window_size;
            } break;
            case SDLK_1: {
                event.type = Event::Type::Decrease_window_size;
            } break;
            case SDLK_SPACE: {
                event.type = Event::Type::Hold;
            } break;
            case SDLK_ESCAPE: {
                event.type = Event::Type::Pause;
            } break;
            default: {
            } break;
            }
        } else if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
            case SDLK_DOWN: {
                event.type = Event::Type::Reset_speed;
            } break;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                event.type = Event::Type::Mousebuttondown;
                event.x = e.button.x;
                event.y = e.button.y;
            }
        }
    }

    return event;
}


auto static init_window() {
    SDL_Init(SDL_INIT_EVERYTHING);

    auto newScale {windowScale};
    {
        Rect<int>::Size dim {};
        do {
            ++newScale;
            dim = {gBaseWindowWidth * newScale, gBaseWindowHeight * newScale};
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
