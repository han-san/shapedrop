#include <cassert>
#include <ctime>
#include <iostream>
#include <array>
#include <vector>

#include <SDL2/SDL.h>
#include <jlib/jint.h>

struct BackBuffer {
    void* memory;
    int w;
    int h;
    int pitch;
    int bpp;
};

struct V2 {
    union {
        int w;
        int x;
    };
    union {
        int h;
        int y;
    };
};

enum Message {
    NONE,
    QUIT,
    RESET,
    MOVE_RIGHT,
    MOVE_LEFT,
};

struct Square {
    float x;
    float y;
    int w;
    int h;
};

auto constexpr rows = 20;
auto constexpr columns = 10;
auto constexpr scale = 40;
auto constexpr windoww = columns * scale;
auto constexpr windowh = rows * scale;
auto gRunning = true;

SDL_Surface* gBBSurface;
SDL_Surface* gWinSurface;
SDL_Window* gWindow;

auto sdl_get_window_dimensions() -> V2
{
    return { windoww, windowh };
}

auto sdl_swap_buffer() -> void
{
    SDL_BlitSurface(gBBSurface, NULL, gWinSurface, NULL);
    SDL_UpdateWindowSurface(gWindow);
}

auto sdl_get_back_buffer() -> BackBuffer
{
    auto bbuf = BackBuffer{};
    bbuf.memory = gBBSurface->pixels;
    bbuf.w = gBBSurface->w;
    bbuf.h = gBBSurface->h;
    bbuf.pitch = gBBSurface->pitch;
    bbuf.bpp = gBBSurface->format->BytesPerPixel;

    return bbuf;
}

auto sdl_handle_input(Message* outMsg) -> bool
{
    *outMsg = NONE;
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *outMsg = QUIT;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_RIGHT: {
                *outMsg = MOVE_RIGHT;
            } break;
            case SDLK_LEFT: {
                *outMsg = MOVE_LEFT;
            } break;
            case SDLK_r: {
                *outMsg = RESET;
            } break;
            default: {
            } break;
            }
        }
    }

    return *outMsg ? true : false;
}

struct Point {
    float x;
    float y;
};

auto draw_solid_square(BackBuffer* buf, Square sqr, uint r, uint g, uint b)
{
    for (auto y = 0; y < sqr.h; ++y) {
        auto pixely = (int)sqr.y + y;
        if (pixely < 0 || pixely >= buf->h) {
            continue;
        }
        for (auto x = 0; x < sqr.w; ++x) {
            auto pixelx = (int)sqr.x + x;
            if (pixelx < 0 || pixelx >= buf->w) {
                continue;
            }

            auto currbyteindex = pixely * buf->w + pixelx;
            auto currbyte = ((u8*)buf->memory + currbyteindex * buf->bpp);

            *currbyte++ = b;
            *currbyte++ = g;
            *currbyte++ = r;
        }
    }
}

auto draw_image(BackBuffer* backBuf, Point dest, BackBuffer* img)
{
    for (auto y = 0; y < img->h; ++y) {
        auto pixely = (int)dest.y + y;
        if (pixely < 0 || pixely >= backBuf->h) {
            continue;
        }
        for (auto x = 0; x < img->w; ++x) {
            auto pixelx = (int)dest.x + x;
            if (pixelx < 0 || pixelx >= backBuf->w) {
                continue;
            }

            auto currBBbyteindex = pixely * backBuf->w + pixelx;
            auto currBBbyte = ((u8*)backBuf->memory + currBBbyteindex * backBuf->bpp);
            auto currimgbyteindex = y * img->w + x;
            auto currimgbyte = ((u8*)img->memory + currimgbyteindex * img->bpp);

            auto r = *currimgbyte++;
            auto g = *currimgbyte++;
            auto b = *currimgbyte++;
            auto a = *currimgbyte++;

            // FIXME: hack
            if (!a) {
                continue;
            }

            *currBBbyte++ = b;
            *currBBbyte++ = g;
            *currBBbyte++ = r;
        }
    }
}


auto guy = Square{};
auto jumpspeed = 0;
auto velocity = 0.0;
auto gravity = 0;

auto scroll = 0.0;
auto scrollspeed = 0;

auto score = 0.0;
auto lost = false;

auto currentclock = (decltype(clock())) 0;
auto delta = 0.0;

auto highScore = 0;

auto init()
{
    auto winDimensions = sdl_get_window_dimensions();
    guy.w = 10;
    guy.h = 10;
    guy.x = winDimensions.w / 3;
    guy.y = winDimensions.h / 3;

    jumpspeed = 600;
    velocity = 0.0;
    gravity = 1500;

    scroll = 0.0;
    scrollspeed = 200;

    score = 0.0;
    lost = false;

    delta = 0.0;
    currentclock = clock();

    srand(time(NULL));
}

auto run() -> void
{
    auto bb = sdl_get_back_buffer();
    gRunning = true;

    auto winDimensions = sdl_get_window_dimensions();

    init();

    std::array<bool, rows * columns> board = {};

    auto is_valid_spot = [&](V2 pos) {
        if (pos.x < 0 || pos.x >= columns || pos.y < 0 || pos.y >= rows) {
            return true;
        } else {
            auto index = pos.y * columns + pos.x;
            return !board[index];
        }
    };

    struct Shape {
        std::vector<V2> blocks;
    };

    auto baseX = 0;
    auto baseY = 0;
    auto LPiece = Shape {
        {
            // l-piece
            { {baseX + 0}, {baseY + 0} },
            /* { {baseX + 0}, {baseY + 1} }, */
            /* { {baseX + 0}, {baseY + 2} }, */
            /* { {baseX + 0}, {baseY + 3} }, */
        }
    };

    auto currentShape = LPiece;

    auto dropclock = clock();

    while (gRunning) {
        auto newclock = clock();
        auto frameclocktime = newclock - currentclock;
        currentclock = newclock;

        delta = (double)frameclocktime / CLOCKS_PER_SEC;
        auto framemstime = 1000.0 * delta;

        // TODO: sleep so cpu doesn't melt

        // input
        auto message = Message{};
        while (sdl_handle_input(&message)) {
            if (message == QUIT) {
                gRunning = false;
            } else if (message == RESET) {
                init();
            } else if (message == MOVE_RIGHT) {
                auto canMove = true;
                for (auto& block : currentShape.blocks) {
                    // check if block to the right is available
                    if (block.x + 1 >= columns) {
                        canMove = false;
                    }
                }
                if (canMove) {
                    for (auto& block : currentShape.blocks) {
                        ++block.x;
                    }
                }
            } else if (message == MOVE_LEFT) {
                auto canMove = true;
                for (auto& block : currentShape.blocks) {
                    // check if block to the left is available
                    if (block.x <= 0) {
                        canMove = false;
                    }
                }
                if (canMove) {
                    for (auto& block : currentShape.blocks) {
                        --block.x;
                    }
                }
            }
        }

        // sim
        {
            // 1 drop per second
            auto nextdropclock = dropclock + 1 * CLOCKS_PER_SEC;
            if (currentclock > nextdropclock) {
                dropclock = currentclock;
                auto canDrop = true;
                for (auto& block : currentShape.blocks) {
                    // check if the block below is occupied or not
                    auto boardIndex = (block.y + 1) * columns + block.x;
                    if (board[boardIndex]) {
                        canDrop = false;
                    } else if (block.y + 1 >= rows) {
                        canDrop = false;
                    }
                }
                if (canDrop) {
                    for (auto& block : currentShape.blocks) {
                        ++block.y;
                    }
                } else {
                    for (auto& block : currentShape.blocks) {
                        auto boardIndex = block.y * columns + block.x;
                        board[boardIndex] = true;
                    }
                    currentShape = LPiece;
                }
            }
        }

        // draw background
        for (auto y = 0; y < rows; ++y) {
            for (auto x = 0; x < columns; ++x) {
                auto currindex = y * columns + x;
                auto color = board[currindex] ? 0xff : 0;
                draw_solid_square(&bb, {float(x * scale), float(y * scale), scale, scale}, color, color, color);
            }
        }

        // draw current shape
        for (auto& block : currentShape.blocks) {
            draw_solid_square(&bb, {float(block.x * scale), float(block.y * scale), scale, scale}, 0xff, 0xff, 0xff);
        }

        sdl_swap_buffer();
    }
}

auto main(int argc, char** argv) -> int{
    SDL_Init(SDL_INIT_EVERYTHING);

    gWindow = SDL_CreateWindow("Tetris",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               windoww, windowh, SDL_WINDOW_SHOWN);
    assert(gWindow);

    gWinSurface = SDL_GetWindowSurface(gWindow);
    assert(gWinSurface);

    gBBSurface = SDL_CreateRGBSurface(0, gWinSurface->w, gWinSurface->h,
                                      gWinSurface->format->BitsPerPixel,
                                      gWinSurface->format->Rmask,
                                      gWinSurface->format->Gmask,
                                      gWinSurface->format->Bmask,
                                      gWinSurface->format->Amask);
    assert(gBBSurface);

    run();

    return 0;
}
