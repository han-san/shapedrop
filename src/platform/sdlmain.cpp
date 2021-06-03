#include "sdlmain.hpp"

#include "../core.hpp"
#include "../font.hpp"
#include "../input.hpp"
#include "../platform.hpp"
#include "../util.hpp"

#include "../jint.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <glad/glad.h> // must be included before SDL

#include <cassert>
#include <optional>
#include <string_view>

namespace platform::SDL {

auto windowScale = 1;

struct {
  SDL_Window* handle {};
  SDL_Surface* surface {};
  SDL_Surface* bbSurface {};
  Rect<int>::Size dimensions {};
} window {};

SDL_GLContext g_glContext {};

OpenGLRender::Context static* context = nullptr;

RenderMode g_renderMode {RenderMode::opengl};

auto get_render_mode() -> RenderMode { return g_renderMode; }

auto get_gl_context() -> SDL_GLContext { return g_glContext; }
auto get_window_scale() -> int { return windowScale; }

auto get_back_buffer() -> BackBuffer {
  auto bbuf = BackBuffer {};
  bbuf.memory = window.bbSurface->pixels;
  bbuf.dimensions = {PositiveUInt {window.bbSurface->w},
                     PositiveUInt {window.bbSurface->h}};
  bbuf.pitch = PositiveUInt {window.bbSurface->pitch};
  bbuf.bpp = window.bbSurface->format->BytesPerPixel;

  return bbuf;
}

auto get_window_dimensions() -> Rect<int>::Size { return window.dimensions; }

auto static resize_window(Rect<int>::Size const dimensions) {
  window.dimensions = dimensions;
  SDL_SetWindowSize(window.handle, dimensions.w, dimensions.h);
  window.surface = SDL_GetWindowSurface(window.handle);
  assert(window.surface);
  SDL_FreeSurface(window.bbSurface);
  window.bbSurface = SDL_CreateRGBSurface(
      0, window.surface->w, window.surface->h,
      window.surface->format->BitsPerPixel, window.surface->format->Rmask,
      window.surface->format->Gmask, window.surface->format->Bmask,
      window.surface->format->Amask);
  assert(window.bbSurface);

  if (get_render_mode() == RenderMode::opengl) {
    glViewport(0, 0, window.dimensions.w, window.dimensions.h);
  }
}

auto change_window_scale(int newScale) -> void {
  if (newScale < 1) {
    newScale = 1;
  }
  if (windowScale == newScale) {
    return;
  }
  windowScale = newScale;
  resize_window(
      {gBaseWindowWidth * windowScale, gBaseWindowHeight * windowScale});
}

auto swap_buffer() -> void {
  switch (get_render_mode()) {
  case RenderMode::software: {
    SDL_BlitSurface(window.bbSurface, nullptr, window.surface, nullptr);
    SDL_UpdateWindowSurface(window.handle);
  } break;
  case RenderMode::opengl: {
    SDL_GL_SwapWindow(window.handle);
  } break;
  }
}

auto static window_fits_on_screen(Rect<int>::Size windowDimensions) -> bool {
  SDL_Rect displayBounds {};
  SDL_GetDisplayUsableBounds(0, &displayBounds);

  return windowDimensions.w < displayBounds.w and
         windowDimensions.h < displayBounds.h;
}

auto get_event() -> Event {
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
        event.mouseCoords = {e.button.x, e.button.y};
      }
    }
  }

  return event;
}

auto init_window_opengl() {
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    throw;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  windowScale = 30;
  window.dimensions = {gBaseWindowWidth * windowScale,
                       gBaseWindowHeight * windowScale};

  window.handle = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED, window.dimensions.w,
                                   window.dimensions.h,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  assert(window.handle);

  g_glContext = SDL_GL_CreateContext(window.handle);
  assert(g_glContext);

  if (gladLoadGLLoader(static_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
    throw;
  }

  glViewport(0, 0, window.dimensions.w, window.dimensions.h);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

auto init_window_software() {
  SDL_Init(SDL_INIT_EVERYTHING);

  auto newScale = windowScale;
  {
    Rect<int>::Size dim {};
    do {
      ++newScale;
      dim = {gBaseWindowWidth * newScale, gBaseWindowHeight * newScale};
    } while (window_fits_on_screen(dim));
  }
  --newScale;
  windowScale = newScale;
  auto const initialWindowWidth = gBaseWindowWidth * windowScale;
  auto const initialWindowHeight = gBaseWindowHeight * windowScale;
  window.dimensions.w = initialWindowWidth;
  window.dimensions.h = initialWindowHeight;

  window.handle = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED, initialWindowWidth,
                                   initialWindowHeight, SDL_WINDOW_SHOWN);
  assert(window.handle);

  window.surface = SDL_GetWindowSurface(window.handle);
  assert(window.surface);

  window.bbSurface = SDL_CreateRGBSurface(
      0, window.surface->w, window.surface->h,
      window.surface->format->BitsPerPixel, window.surface->format->Rmask,
      window.surface->format->Gmask, window.surface->format->Bmask,
      window.surface->format->Amask);
  assert(window.bbSurface);
}

auto static init_window(RenderMode renderMode) {
  switch (renderMode) {
  case RenderMode::opengl: {
    init_window_opengl();
  } break;
  case RenderMode::software: {
    init_window_software();
  } break;
  }
}

auto static destroy_window() {
  SDL_DestroyWindow(window.handle);
  window.handle = nullptr;
  SDL_Quit();
}

auto get_opengl_render_context() -> OpenGLRender::Context const& {
  return *context;
}

} // namespace platform::SDL

auto main(int argc, char** argv) -> int {
  for (int i = 1; i < argc; ++i) {
    std::string_view arg {argv[i]};
    using namespace std::string_view_literals;
    if (arg == "-opengl"sv) {
      g_renderMode = RenderMode::opengl;
    } else if (arg == "-software"sv) {
      g_renderMode = RenderMode::software;
    }
  }

  init_window(g_renderMode);
  std::optional<OpenGLRender::Context> openglRenderContext = std::nullopt;
  if (g_renderMode == RenderMode::opengl) {
    openglRenderContext = OpenGLRender::Context {};
    context = &(*openglRenderContext);
  }

  if (not init_font("DejaVuSans.ttf")) {
    assert(false);
    return 1;
  }

  run();

  platform::SDL::destroy_window();

  return 0;
}
