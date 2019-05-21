#include <cassert>

#include <SDL2/SDL.h>

#include "sdlmain.hpp"
#include "core.hpp"

namespace platform::SDL {

auto scale = 1;

struct {
    SDL_Window* handle;
    SDL_Surface* surface;
    SDL_Surface* bbSurface;
} window = {};

auto get_window_scale() -> int
{
    return scale;
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
    if (scale == newScale) return;
    scale = newScale;
    resize_window({baseWindowWidth * scale, baseWindowHeight * scale});
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

    auto newScale = scale;
    {
        auto dim = V2 {};
        do {
            ++newScale;
            dim = V2 {baseWindowWidth * newScale, baseWindowHeight * newScale};
        } while (window_fits_on_screen(dim));
    }
    --newScale;
    scale = newScale;
    auto initialWindowWidth = baseWindowWidth * scale;
    auto initialWindowHeight = baseWindowHeight * scale;

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

    run();
}

