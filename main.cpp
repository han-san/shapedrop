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

    V2(int a = 0, int b = 0): x{a}, y{b} {}
};

struct V3 {
    union {
        int x;
        int r;
    };
    union {
        int y;
        int g;
    };
    union {
        int z;
        int b;
    };

    V3(int a = 0, int b = 0, int c = 0): x{a}, y{b}, z{c} {}
};

enum class Message {
    NONE,
    QUIT,
    RESET,
    MOVE_RIGHT,
    MOVE_LEFT,
    INCREASE_SPEED,
    RESET_SPEED,
    DROP,
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
auto constexpr windoww = (columns + 2) * scale;
auto constexpr windowh = (rows + 2) * scale;
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

auto sdl_handle_input() -> Message
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

auto dropSpeed = 1.0;
auto maxDropSpeed = 0.1;

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

    using Position = V2;
    using Color = V3;

    struct Block {
        Position pos;
        Color color;
        bool isActive = false;
    };

    std::array<Block, rows * columns> board = {};

    auto is_valid_spot = [&](V2 pos) {
        if (pos.x < 0 || pos.x >= columns || pos.y < 0 || pos.y >= rows) {
            return true;
        } else {
            auto index = pos.y * columns + pos.x;
            return !board[index].isActive;
        }
    };

    struct Shape {
        std::vector<Block> blocks;

        Shape(Color color, std::initializer_list<Position> positions) {
            blocks.reserve(positions.size());
            for (auto& pos : positions) {
                blocks.push_back({ pos, color, true });
            }
        }
    };

    auto baseX = columns / 2;
    auto baseY = 0;
    auto IPiece = Shape(Color(0x00, 0xf0, 0xf0), {
                            { baseX + 0, baseY + 0 },
                            { baseX + 0, baseY + 1 },
                            { baseX + 0, baseY + 2 },
                            { baseX + 0, baseY + 3 }
                        });
    auto LPiece = Shape(Color(0xf0, 0xa0, 0x00), {
                            { baseX + 0, baseY + 0 },
                            { baseX + 0, baseY + 1 },
                            { baseX + 0, baseY + 2 },
                            { baseX + 1, baseY + 2 },
                        });
    auto JPiece = Shape(Color(0x00, 0x00, 0xf0), {
                            { baseX + 1, baseY + 0 },
                            { baseX + 1, baseY + 1 },
                            { baseX + 1, baseY + 2 },
                            { baseX + 0, baseY + 2 },
                        });
    auto OPiece = Shape(Color(0xf0, 0xf0, 0x00), {
                            { baseX + 0, baseY + 0 },
                            { baseX + 1, baseY + 0 },
                            { baseX + 1, baseY + 1 },
                            { baseX + 0, baseY + 1 },
                        });
    auto SPiece = Shape(Color(0x00, 0xf0, 0x00), {
                            { baseX + 1, baseY + 0 },
                            { baseX + 2, baseY + 0 },
                            { baseX + 1, baseY + 1 },
                            { baseX + 0, baseY + 1 },
                        });
    auto ZPiece = Shape(Color(0xf0, 0x00, 0x00), {
                            { baseX + 0, baseY + 0 },
                            { baseX + 1, baseY + 0 },
                            { baseX + 1, baseY + 1 },
                            { baseX + 2, baseY + 1 },
                        });
    auto TPiece = Shape(Color(0xa0, 0x00, 0xf0), {
                            { baseX + 0, baseY + 0 },
                            { baseX + 1, baseY + 0 },
                            { baseX + 1, baseY + 1 },
                            { baseX + 2, baseY + 0 },
                        });
    std::array<Shape, 7> shapes = {
        IPiece,
        LPiece,
        JPiece,
        OPiece,
        SPiece,
        ZPiece,
        TPiece,
    };

    auto currentShape = shapes[0];

    auto dropclock = clock();

    while (gRunning) {
        auto newclock = clock();
        auto frameclocktime = newclock - currentclock;
        currentclock = newclock;

        delta = (double)frameclocktime / CLOCKS_PER_SEC;
        auto framemstime = 1000.0 * delta;

        // TODO: sleep so cpu doesn't melt

        // input
        Message message;
        while ((message = sdl_handle_input()) != Message::NONE) {
            if (message == Message::QUIT) {
                gRunning = false;
            } else if (message == Message::RESET) {
                init();
            } else if (message == Message::MOVE_RIGHT) {
                auto canMove = true;
                for (auto& block : currentShape.blocks) {
                    // check if block to the right is available
                    auto x = block.pos.x + 1;
                    auto boardIndex = block.pos.y * columns + x;
                    if (x >= columns || board[boardIndex].isActive) {
                        canMove = false;
                    }
                }
                if (canMove) {
                    for (auto& block : currentShape.blocks) {
                        ++block.pos.x;
                    }
                    // reset drop clock
                    dropclock = currentclock;
                }
            } else if (message == Message::MOVE_LEFT) {
                auto canMove = true;
                for (auto& block : currentShape.blocks) {
                    // check if block to the left is available
                    auto x = block.pos.x - 1;
                    auto boardIndex = block.pos.y * columns + x;
                    if (block.pos.x <= 0 || board[boardIndex].isActive) {
                        canMove = false;
                    }
                }
                if (canMove) {
                    for (auto& block : currentShape.blocks) {
                        --block.pos.x;
                    }
                    // reset drop clock
                    dropclock = currentclock;
                }
            } else if (message == Message::INCREASE_SPEED) {
                dropSpeed = maxDropSpeed;
            } else if (message == Message::RESET_SPEED) {
                dropSpeed = 1.0;

                // reset drop clock
                dropclock = currentclock;
            } else if (message == Message::DROP) {
                auto canDrop = true;
                while (canDrop) {
                    for (auto& block : currentShape.blocks) {
                        // check if the block below is occupied or not
                        auto boardIndex = (block.pos.y + 1) * columns + block.pos.x;
                        if (board[boardIndex].isActive) {
                            canDrop = false;
                        } else if (block.pos.y + 1 >= rows) {
                            canDrop = false;
                        }
                    }
                    if (canDrop) {
                        for (auto& block : currentShape.blocks) {
                            ++block.pos.y;
                        }
                    }
                }

                // reset drop clock
                dropclock = currentclock;
            }
        }

        // sim
        {
            // 1 drop per second
            auto nextdropclock = dropclock + dropSpeed * CLOCKS_PER_SEC;
            if (currentclock > nextdropclock) {
                dropclock = currentclock;
                auto canDrop = true;
                for (auto& block : currentShape.blocks) {
                    // check if the block below is occupied or not
                    auto boardIndex = (block.pos.y + 1) * columns + block.pos.x;
                    if (board[boardIndex].isActive) {
                        canDrop = false;
                    } else if (block.pos.y + 1 >= rows) {
                        canDrop = false;
                    }
                }
                if (canDrop) {
                    for (auto& block : currentShape.blocks) {
                        ++block.pos.y;
                    }
                } else {
                    for (auto& block : currentShape.blocks) {
                        auto boardIndex = block.pos.y * columns + block.pos.x;
                        board[boardIndex] = block;
                    }
                    currentShape = shapes[rand() % shapes.size()];
                }
            }
        }

        // draw border
        for (auto y = 0; y < windowh; ++y) {
            for (auto x = 0; x < windoww; ++x) {
                draw_solid_square(&bb, {float(x), float(y), 1, 1}, 0xff * (float(x) / windoww), 0xff * (1 - (float(x) / windoww) * (float(y) / windowh)), 0xff * (float(y) / windowh));
            }
        }

        // draw background
        for (auto y = 0; y < rows; ++y) {
            for (auto x = 0; x < columns; ++x) {
                auto currindex = y * columns + x;
                auto& block = board[currindex];
                auto color = block.isActive ? block.color : Color { 0, 0, 0 };
                draw_solid_square(&bb, {float((x + 1) * scale), float((y + 1) * scale), scale, scale}, color.r, color.g, color.b);
            }
        }

        // draw shadow (could be calculated once when moved horizontally or rotated instead of every frame)
        auto currentShapeShadow = currentShape;
        auto canDrop = true;
        while (canDrop) {
            for (auto& block : currentShapeShadow.blocks) {
                // check if the block below is occupied or not
                auto boardIndex = (block.pos.y + 1) * columns + block.pos.x;
                if (board[boardIndex].isActive) {
                    canDrop = false;
                } else if (block.pos.y + 1 >= rows) {
                    canDrop = false;
                }
            }
            if (canDrop) {
                for (auto& block : currentShapeShadow.blocks) {
                    ++block.pos.y;
                }
            }
        }

        for (auto& block : currentShapeShadow.blocks) {
            draw_solid_square(&bb, {float((block.pos.x + 1) * scale), float((block.pos.y + 1) * scale), scale, scale}, 0x20, 0x20, 0x20);
        }

        // draw current shape
        for (auto& block : currentShape.blocks) {
            draw_solid_square(&bb, {float((block.pos.x + 1) * scale), float((block.pos.y + 1) * scale), scale, scale}, block.color.r, block.color.g, block.color.b);
        }

        sdl_swap_buffer();
    }
}

auto main(int argc, char** argv) -> int {
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
