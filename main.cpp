#include <cassert>
#include <algorithm>
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
    ROTATE_LEFT,
    ROTATE_RIGHT,
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
            case SDLK_z: {
                msg = Message::ROTATE_LEFT;
            } break;
            case SDLK_x: {
                msg = Message::ROTATE_RIGHT;
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
auto dropclock = (decltype(clock())) 0;

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
    dropclock = currentclock;

    srand(time(NULL));
}

auto run() -> void
{
    auto bb = sdl_get_back_buffer();
    gRunning = true;

    /* auto winDimensions = sdl_get_window_dimensions(); */

    init();

    using Position = V2;
    using Color = V3;

    struct Block {
        Position pos;
        Color color;
        bool isActive = false;
    };

    std::array<Block, rows * columns> board = {};

    struct Shape {
        std::vector<Block> blocks;

        Shape(Color color, std::initializer_list<Position> positions) {
            blocks.reserve(positions.size());
            for (auto& pos : positions) {
                blocks.push_back({ pos, color, true });
            }
        }
    };

    auto baseX = columns / 2 - 1;
    auto baseY = -2;
    auto IPiece = Shape(Color(0x00, 0xf0, 0xf0), {
                            { baseX - 1, baseY + 1 },
                            { baseX + 0, baseY + 1 },
                            { baseX + 1, baseY + 1 },
                            { baseX + 2, baseY + 1 }
                        });
    auto LPiece = Shape(Color(0xf0, 0xa0, 0x00), {
                            { baseX - 1, baseY + 1 },
                            { baseX + 0, baseY + 1 },
                            { baseX + 1, baseY + 1 },
                            { baseX + 1, baseY + 0 },
                        });
    auto JPiece = Shape(Color(0x00, 0x00, 0xf0), {
                            { baseX - 1, baseY + 0 },
                            { baseX - 1, baseY + 1 },
                            { baseX + 0, baseY + 1 },
                            { baseX + 1, baseY + 1 },
                        });
    auto OPiece = Shape(Color(0xf0, 0xf0, 0x00), {
                            { baseX + 0, baseY + 0 },
                            { baseX + 1, baseY + 0 },
                            { baseX + 1, baseY + 1 },
                            { baseX + 0, baseY + 1 },
                        });
    auto SPiece = Shape(Color(0x00, 0xf0, 0x00), {
                            { baseX + 1, baseY + 0 },
                            { baseX + 0, baseY + 0 },
                            { baseX + 0, baseY + 1 },
                            { baseX - 1, baseY + 1 },
                        });
    auto ZPiece = Shape(Color(0xf0, 0x00, 0x00), {
                            { baseX + 0, baseY + 0 },
                            { baseX - 1, baseY + 0 },
                            { baseX + 0, baseY + 1 },
                            { baseX + 1, baseY + 1 },
                        });
    auto TPiece = Shape(Color(0xa0, 0x00, 0xf0), {
                            { baseX - 1, baseY + 1 },
                            { baseX + 0, baseY + 1 },
                            { baseX + 0, baseY + 0 },
                            { baseX + 1, baseY + 1 },
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

    class ShapePool {
        std::array<const Shape*, 7> shapePool;
        decltype(shapePool) previewPool;
        decltype(shapePool.begin()) currentShapeIterator;

    public:
        ShapePool(const std::array<Shape, 7>& shapes) {
            shapePool = {
                &shapes[0], &shapes[1], &shapes[2],
                &shapes[3], &shapes[4], &shapes[5],
                &shapes[6],
            };
            previewPool = shapePool;

            std::random_shuffle(shapePool.begin(), shapePool.end());
            std::random_shuffle(previewPool.begin(), previewPool.end());
            currentShapeIterator = shapePool.begin();
        }

        auto next_shape() -> Shape {
            ++currentShapeIterator;
            if (currentShapeIterator == shapePool.end()) {
                shapePool = previewPool;
                currentShapeIterator = shapePool.begin();
                std::random_shuffle(previewPool.begin(), previewPool.end());
            }
            return **currentShapeIterator;
        }

        auto current_shape() -> Shape {
            return **currentShapeIterator;
        }

    };
    ShapePool shapePool{shapes};

    auto currentShape = shapePool.current_shape();

    auto is_valid_spot = [&board](V2 pos) {
        if (pos.x < 0 || pos.x >= columns || pos.y < 0 || pos.y >= rows) {
            return false;
        } else {
            auto index = pos.y * columns + pos.x;
            return !board[index].isActive;
        }
    };

    auto is_valid_move = [is_valid_spot](Shape& shape, V2 move) {
        for (auto& block : shape.blocks) {
            auto x = block.pos.x + move.x;
            auto y = block.pos.y + move.y;
            if (!is_valid_spot({x, y})) {
                return false;
            }
        }
        return true;
    };

    auto try_move = [is_valid_move](Shape& shape, V2 move) {
        if (is_valid_move(shape, move)) {
            for (auto& block : shape.blocks) {
                block.pos.x += move.x;
                block.pos.y += move.y;
            }
            dropclock = currentclock;
        }
    };

    auto remove_full_rows = [](std::array<Block, rows * columns>& board) {
        // check if a row can be cleared
        // a maximum of 4 rows can be cleared at once with default shapes
        class {
            std::array<int, 4> m_data = {};
            size_t m_size = 0;
        public:
            int& front() { return m_data[0]; }
            int& back() { return m_data[m_size - 1]; }
            size_t size() { return m_size; }
            void push(int i) { m_data[m_size++] = i; }
            bool empty() { return !m_size; }
        } rowsCleared;

        for (auto y = 0; y < rows; ++y) {
            auto rowIsFull = true;
            for (auto x = 0; x < columns; ++x) {
                auto boardIndex = y * columns + x;
                if (!board[boardIndex].isActive) {
                    rowIsFull = false;
                    break;
                }
            }
            if (rowIsFull) {
                rowsCleared.push(y);
            }
        }
        assert(rowsCleared.size() <= 4);

        if (!rowsCleared.empty()) {
            auto topRow = rowsCleared.front();
            auto botRow = rowsCleared.back();
            assert(botRow >= topRow);

            // remove rows
            for (auto y = topRow; y <= botRow; ++y) {
                for (auto x = 0; x < columns; ++x) {
                    auto index = y * columns + x;
                    board[index].isActive = false;
                }
            }

            // move rows above removed rows
            for (auto y = topRow - 1; y >= 0; --y) {
                for (auto x = 0; x < columns; ++x) {
                    auto index = y * columns + x;
                    auto newIndex = (y + rowsCleared.size()) * columns + x;
                    auto& oldBlock = board[index];
                    auto& newBlock = board[newIndex];
                    if (oldBlock.isActive) {
                        newBlock = oldBlock;
                        oldBlock.isActive = false;
                    }
                }
            }
        }
    };

    while (gRunning) {
        auto newclock = clock();
        auto frameclocktime = newclock - currentclock;
        currentclock = newclock;

        delta = (double)frameclocktime / CLOCKS_PER_SEC;
        /* auto framemstime = 1000.0 * delta; */

        // TODO: sleep so cpu doesn't melt

        // input
        Message message;
        while ((message = sdl_handle_input()) != Message::NONE) {
            if (message == Message::QUIT) {
                gRunning = false;
            } else if (message == Message::RESET) {
                init();
            } else if (message == Message::MOVE_RIGHT) {
                try_move(currentShape, {1, 0});
            } else if (message == Message::MOVE_LEFT) {
                try_move(currentShape, {-1, 0});
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
            } else if (message == Message::ROTATE_LEFT) {
                auto xSum = 0;
                auto ySum = 0;
                for (auto& block : currentShape.blocks) {
                    xSum += block.pos.x;
                    ySum += block.pos.y;
                }
                auto xAvg = xSum / currentShape.blocks.size();
                auto yAvg = ySum / currentShape.blocks.size();

                for (auto& block : currentShape.blocks) {
                    auto x = block.pos.x;
                    auto y = block.pos.y;
                    block.pos.y = yAvg + (xAvg - x);
                    block.pos.x = xAvg - (yAvg - y);
                }
            } else if (message == Message::ROTATE_RIGHT) {
                auto xSum = 0;
                auto ySum = 0;
                for (auto& block : currentShape.blocks) {
                    xSum += block.pos.x;
                    ySum += block.pos.y;
                }
                auto xAvg = xSum / currentShape.blocks.size();
                auto yAvg = ySum / currentShape.blocks.size();

                for (auto& block : currentShape.blocks) {
                    auto x = block.pos.x;
                    auto y = block.pos.y;
                    block.pos.y = yAvg - (xAvg - x);
                    block.pos.x = xAvg + (yAvg - y);
                }
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
                    // fix currentBlocks position on board
                    for (auto& block : currentShape.blocks) {
                        auto boardIndex = block.pos.y * columns + block.pos.x;
                        board[boardIndex] = block;
                    }

                    remove_full_rows(board);

                    currentShape = shapePool.next_shape();

                    // game over if there is block occupying spawn location
                    auto gameOver = false;
                    for (auto& block : currentShape.blocks) {
                        if (!is_valid_spot(block.pos)) {
                            gameOver = true;
                            break;
                        }
                    }

                    if (gameOver) {
                        std::cout << "Game Over!\n";
                    }
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
    if (argc || argv) {}
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
