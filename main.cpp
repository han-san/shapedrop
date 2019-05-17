#include <cassert>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <array>

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

using Position = V2;
using Color = V3;

struct Block {
    Color color;
    bool isActive = false;
};

auto constexpr rows = 22;
auto constexpr columns = 10;

using Board = std::array<Block, rows * columns>;

auto is_valid_spot(Board& board, V2 pos) -> bool {
    if (pos.x < 0 || pos.x >= columns || pos.y < 0 || pos.y >= rows) {
        return false;
    } else {
        auto index = pos.y * columns + pos.x;
        return !board[index].isActive;
    }
}

template <typename T, size_t I>
class ArrayStack {
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const pointer;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    auto constexpr begin() noexcept -> iterator { return &m_data[0]; }
    auto constexpr begin() const noexcept -> const_iterator { return &m_data[0]; }
    auto constexpr end() noexcept -> iterator { return &m_data[m_size]; }
    auto constexpr end() const noexcept -> const_iterator { return &m_data[m_size]; }
    auto constexpr cbegin() const noexcept -> const_iterator { return &m_data[0]; }
    auto constexpr cend() const noexcept -> const_iterator { return &m_data[m_size]; }
    auto constexpr front() -> reference { return m_data[0]; }
    auto constexpr front() const -> const_reference { return m_data[0]; }
    auto constexpr back() -> reference { return m_data[m_size - 1]; }
    auto constexpr back() const -> const_reference { return m_data[m_size - 1]; }
    auto constexpr size() noexcept -> size_type { return m_size; }
    auto constexpr max_size() noexcept -> size_type { return I; }
    auto constexpr push_back(value_type i) -> void { m_data[m_size++] = i; }
    auto constexpr pop_back() -> void { m_data[--m_size].~T(); }
    auto constexpr empty() -> bool { return !m_size; }

private:
    std::array<value_type, I> m_data = {};
    size_type m_size = 0;
};

struct Shape {
    using ShapeLayout = std::array<bool, 16>;
    using RotationMap = std::array<ShapeLayout, 4>;

    RotationMap static constexpr IRotationMap = {
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 1, 1,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 1, 0,
            0, 0, 1, 0,
            0, 0, 1, 0,
            0, 0, 1, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            0, 0, 0, 0,
            1, 1, 1, 1,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
        },
    };
    RotationMap static constexpr LRotationMap = {
        ShapeLayout {
            0, 0, 1, 0,
            1, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 1, 0,
            1, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            1, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr JRotationMap = {
        ShapeLayout {
            1, 0, 0, 0,
            1, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 1, 0,
            0, 0, 1, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 0, 0,
            1, 1, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr ORotationMap = {
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr SRotationMap = {
        ShapeLayout {
            0, 1, 1, 0,
            1, 1, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 1, 0,
            0, 0, 1, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            0, 1, 1, 0,
            1, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            1, 0, 0, 0,
            1, 1, 0, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr ZRotationMap = {
        ShapeLayout {
            1, 1, 0, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 1, 0,
            0, 1, 1, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 0, 0,
            0, 1, 1, 1,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            1, 1, 0, 0,
            1, 0, 0, 0,
            0, 0, 0, 0,
        },
    };
    RotationMap static constexpr TRotationMap = {
        ShapeLayout {
            0, 1, 0, 0,
            1, 1, 1, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            0, 1, 1, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 0, 0, 0,
            1, 1, 1, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
        ShapeLayout {
            0, 1, 0, 0,
            1, 1, 0, 0,
            0, 1, 0, 0,
            0, 0, 0, 0,
        },
    };

    enum class Type {
        I, O, L, J, S, Z, T
    };

    Type type;
    RotationMap const* rotations = nullptr;
    int rotationIndex = 0;
    Position pos = columns / 2 - 2; // spawn centrally
    Color color;

    Shape(Type type) {
        this->type = type;
        switch (type) {
            case Type::I: {
                color = Color(0x00, 0xf0, 0xf0);
                rotations = &IRotationMap;
            } break;
            case Type::O: {
                color = Color(0xf0, 0xf0, 0x00);
                rotations = &ORotationMap;
            } break;
            case Type::L: {
                color = Color(0xf0, 0xa0, 0x00);
                rotations = &LRotationMap;
            } break;
            case Type::J: {
                color = Color(0x00, 0x00, 0xf0);
                rotations = &JRotationMap;
            } break;
            case Type::S: {
                color = Color(0x00, 0xf0, 0x00);
                rotations = &SRotationMap;
            } break;
            case Type::Z: {
                color = Color(0xf0, 0x00, 0x00);
                rotations = &ZRotationMap;
            } break;
            case Type::T: {
                color = Color(0xa0, 0x00, 0xf0);
                rotations = &TRotationMap;
            } break;
            default: {
                // shouldn't be possible
                assert(false);
            } break;
        }
    }

    auto get_block_positions() -> ArrayStack<Position, 4> {
        ArrayStack<Position, 4> positions = {};
        auto& layout = (*rotations)[rotationIndex];
        auto constexpr size = 4;
        for (auto y = 0; y < size; ++y) {
            for (auto x = 0; x < size; ++x) {
                auto index = y * size + x;
                if (layout[index]) {
                    positions.push_back({x, y});
                    if (positions.size() == 4) {
                        return positions;
                    }
                }
            }
        }
        assert(false);
    }

    auto get_absolute_block_positions() -> ArrayStack<Position, 4> {
        auto positions = get_block_positions();
        for (auto& position : positions) {
            position.x += pos.x;
            position.y += pos.y;
        }
        return positions;
    }

    auto is_valid(Board& board) -> bool {
        for (auto position : get_absolute_block_positions()) {
            if (!is_valid_spot(board, position)) {
                return false;
            }
        }
        return true;
    }

    enum class Rotation {
        LEFT,
        RIGHT
    };

    auto rotate(Board& board, Rotation dir) -> bool {
        auto rotatingShape = *this;
        rotatingShape.rotationIndex += dir == Rotation::RIGHT ? 1 : -1;
        if (rotatingShape.rotationIndex == -1) rotatingShape.rotationIndex = 3;
        else if (rotatingShape.rotationIndex == 4) rotatingShape.rotationIndex = 0;
        if (rotatingShape.is_valid(board)) {
            *this = rotatingShape;
            return true;
        }

        std::array<V2, 4> kicks = {};

        // do wall kicks to see if valid
        switch (type) {
            case Type::J:
            case Type::L:
            case Type::S:
            case Type::T:
            case Type::Z: {
                switch (rotationIndex) {
                    case 0: {
                        if (dir == Rotation::RIGHT) {
                            kicks = { V2 {-1, 0}, {-1, 1}, {0, -2}, {-1, -2} };
                        } else {
                            kicks = { V2 {1, 0}, {1, 1}, {0, -2}, {1, -2} };
                        }
                    } break;
                    case 1: {
                        // both directions check same positions
                        kicks = { V2 {1, 0}, {1, -1}, {0, 2}, {1, 2} };
                    } break;
                    case 2: {
                        if (dir == Rotation::RIGHT) {
                            kicks = { V2 {1, 0}, {1, 1}, {0, -2}, {1, -2} };
                        } else {
                            kicks = { V2 {-1, 0}, {-1, 1}, {0, -2}, {-1, -2} };
                        }
                    } break;
                    case 3: {
                        // both directions check same positions
                        kicks = { V2 {-1, 0}, {-1, -1}, {0, 2}, {-1, 2} };
                    } break;
                    default: {
                        assert(false);
                    } break;
                }
            } break;
            case Type::I: {
                switch (rotationIndex) {
                    case 0: {
                        if (dir == Rotation::RIGHT) {
                            kicks = { V2 {-2, 0}, {1, 0}, {-2, -1}, {1, 2} };
                        } else {
                            kicks = { V2 {-1, 0}, {2, 0}, {-1, 2}, {2, -1} };
                        }
                    } break;
                    case 1: {
                        if (dir == Rotation::RIGHT) {
                            kicks = { V2 {-1, 0}, {2, 0}, {-1, 2}, {2, -1} };
                        } else {
                            kicks = { V2 {2, 0}, {-1, 0}, {2, 1}, {-1, -2} };
                        }
                    } break;
                    case 2: {
                        if (dir == Rotation::RIGHT) {
                            kicks = { V2 {2, 0}, {-1, 0}, {2, 1}, {-1, -2} };
                        } else {
                            kicks = { V2 {1, 0}, {-2, 0}, {1, -2}, {-2, 1} };
                        }
                    } break;
                    case 3: {
                        if (dir == Rotation::RIGHT) {
                            kicks = { V2 {1, 0}, {-2, 0}, {1, -2}, {-2, 1} };
                        } else {
                            kicks = { V2 {-2, 0}, {1, 0}, {-2, -1}, {1, 2} };
                        }
                    } break;
                    default: {
                        assert(false);
                    } break;
                }

            } break;
            case Type::O: {
                // should have already returned true in the is_valid() check
                assert(false);
            } break;
            default: {
                assert(false);
            }
        }

        for (auto kickMove : kicks) {
            rotatingShape.pos = pos;
            rotatingShape.pos.x += kickMove.x;
            // the y in kicks is bottom up while it's top down for the shape position
            // so we have to invert it by subtracting instead of adding
            rotatingShape.pos.y -= kickMove.y;
            if (rotatingShape.is_valid(board)) {
                *this = rotatingShape;
                return true;
            }
        }
        return false;
    }
};

auto is_valid_move(Board& board, Shape& shape, V2 move) -> bool {
    shape.pos.x += move.x;
    shape.pos.y += move.y;
    auto valid = shape.is_valid(board);
    shape.pos.x -= move.x;
    shape.pos.y -= move.y;
    return valid;
}

auto try_move(Board& board, Shape& shape, V2 move) {
    if (is_valid_move(board, shape, move)) {
        shape.pos.x += move.x;
        shape.pos.y += move.y;
        return true;
    }
    return false;
}

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
    INCREASE_WINDOW_SIZE,
    DECREASE_WINDOW_SIZE,
};

struct Square {
    float x;
    float y;
    int w;
    int h;
};

auto constexpr baseWindowWidth = columns + 2;
auto constexpr baseWindowHeight = rows + 2;
auto scale = 1;
auto gRunning = true;

struct {
    SDL_Window* handle;
    SDL_Surface* surface;
    SDL_Surface* bbSurface;
    V2 dim;
    BackBuffer bb;
} window = {};

auto sdl_swap_buffer() -> void
{
    SDL_BlitSurface(window.bbSurface, NULL, window.surface, NULL);
    SDL_UpdateWindowSurface(window.handle);
}

auto sdl_get_back_buffer() -> BackBuffer
{
    auto bbuf = BackBuffer{};
    bbuf.memory = window.bbSurface->pixels;
    bbuf.w = window.bbSurface->w;
    bbuf.h = window.bbSurface->h;
    bbuf.pitch = window.bbSurface->pitch;
    bbuf.bpp = window.bbSurface->format->BytesPerPixel;

    return bbuf;
}

auto window_fits_on_screen(V2 windowDimensions) -> bool {
    SDL_Rect displayBounds = {};
    SDL_GetDisplayUsableBounds(0, &displayBounds);

    return windowDimensions.w < displayBounds.w && windowDimensions.h < displayBounds.h;
}

auto resize_window(V2 dimensions) {
    SDL_SetWindowSize(window.handle, dimensions.w, dimensions.h);
    window.dim.w = dimensions.w;
    window.dim.h = dimensions.h;

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
    window.bb = sdl_get_back_buffer();
}

auto change_window_scale(int newScale) {
    if (newScale < 1) newScale = 1;
    if (scale == newScale) return;
    scale = newScale;
    resize_window({baseWindowWidth * scale, baseWindowHeight * scale});
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

struct Point {
    float x;
    float y;
};

auto draw_solid_square(BackBuffer* buf, Square sqr, uint r, uint g, uint b, uint a = 0xff)
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

            auto alpha_blend = [](uint bg, uint fg, uint alpha) {
                auto alphaRatio = alpha / 255.0;
                return fg * alphaRatio + bg * (1 - alphaRatio);
            };

            *currbyte = alpha_blend(*currbyte, b, a);
            ++currbyte;
            *currbyte = alpha_blend(*currbyte, g, a);
            ++currbyte;
            *currbyte = alpha_blend(*currbyte, r, a);
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
    guy.w = 10;
    guy.h = 10;
    guy.x = window.dim.w / 3;
    guy.y = window.dim.h / 3;

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
    window.bb = sdl_get_back_buffer();
    gRunning = true;

    init();

    Board board = {};

    auto remove_full_rows = [](std::array<Block, rows * columns>& board) {
        // check if a row can be cleared
        // a maximum of 4 rows can be cleared at once with default shapes
        ArrayStack<int, 4> rowsCleared;

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
                rowsCleared.push_back(y);
            }
        }
        assert(rowsCleared.size() <= 4);

        if (!rowsCleared.empty()) {
            auto topRow = rowsCleared.front();
            auto botRow = rowsCleared.back();
            assert(topRow >= 0 && topRow < rows);
            assert(botRow >= 0 && botRow < rows);
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
                    assert((y + rowsCleared.size()) < rows);
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

    std::array<Shape, 7> const shapes = {
        Shape(Shape::Type::I),
        Shape(Shape::Type::L),
        Shape(Shape::Type::J),
        Shape(Shape::Type::O),
        Shape(Shape::Type::S),
        Shape(Shape::Type::Z),
        Shape(Shape::Type::T),
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

    auto calculateShapeShadow = [&board](Shape& shape) {
        auto shapeShadow = shape;
        while (is_valid_move(board, shapeShadow, {0, 1})) {
            ++shapeShadow.pos.y;
        }
        return shapeShadow;
    };

    auto currentShape = shapePool.current_shape();
    auto currentShapeShadow = calculateShapeShadow(currentShape);

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
                if (try_move(board, currentShape, {1, 0})) {
                    dropclock = currentclock;
                    // update shape shadow
                    currentShapeShadow = calculateShapeShadow(currentShape);
                }
            } else if (message == Message::MOVE_LEFT) {
                if (try_move(board, currentShape, {-1, 0})) {
                    dropclock = currentclock;
                    // update shape shadow
                    currentShapeShadow = calculateShapeShadow(currentShape);
                }
            } else if (message == Message::INCREASE_WINDOW_SIZE) {
                change_window_scale(scale + 1);
            } else if (message == Message::DECREASE_WINDOW_SIZE) {
                change_window_scale(scale - 1);
            } else if (message == Message::INCREASE_SPEED) {
                dropSpeed = maxDropSpeed;
            } else if (message == Message::RESET_SPEED) {
                dropSpeed = 1.0;

                // reset drop clock
                dropclock = currentclock;
            } else if (message == Message::DROP) {
                while (is_valid_move(board, currentShape, {0, 1})) {
                    ++currentShape.pos.y;
                }

                // reset drop clock
                dropclock = currentclock;
            } else if (message == Message::ROTATE_LEFT) {
                if (currentShape.rotate(board, Shape::Rotation::LEFT)) {
                    // update shape shadow
                    currentShapeShadow = calculateShapeShadow(currentShape);
                }
            } else if (message == Message::ROTATE_RIGHT) {
                if (currentShape.rotate(board, Shape::Rotation::RIGHT)) {
                    // update shape shadow
                    currentShapeShadow = calculateShapeShadow(currentShape);
                }
            }
        }

        // sim
        {
            // 1 drop per second
            auto nextdropclock = dropclock + dropSpeed * CLOCKS_PER_SEC;
            if (currentclock > nextdropclock) {
                dropclock = currentclock;
                if (is_valid_move(board, currentShape, {0, 1})) {
                    ++currentShape.pos.y;
                } else {
                    // game over if there is block occupying spawn location
                    auto gameOver = !currentShape.is_valid(board);

                    if (gameOver) {
                        std::cout << "Game Over!\n";
                        continue;
                    }

                    // fix currentBlocks position on board
                    for (auto position : currentShape.get_absolute_block_positions()) {
                        assert(is_valid_spot(board, position));
                        auto boardIndex = position.y * columns + position.x;
                        board[boardIndex] = {currentShape.color, true};
                    }

                    remove_full_rows(board);
                    currentShape = shapePool.next_shape();
                    // update shape shadow
                    currentShapeShadow = calculateShapeShadow(currentShape);
                }
            }
        }

        // draw border
        for (auto y = 0; y < window.dim.h; ++y) {
            for (auto x = 0; x < window.dim.w; ++x) {
                draw_solid_square(&window.bb, {float(x), float(y), 1, 1}, 0xff * (float(x) / window.dim.w), 0xff * (1 - (float(x) / window.dim.w) * (float(y) / window.dim.h)), 0xff * (float(y) / window.dim.h));
            }
        }

        // draw background
        for (auto y = 2; y < rows; ++y) {
            for (auto x = 0; x < columns; ++x) {
                auto currindex = y * columns + x;
                auto& block = board[currindex];
                auto color = block.isActive ? block.color : Color { 0, 0, 0 };
                draw_solid_square(&window.bb, {float((x + 1) * scale), float((y + 1) * scale), scale, scale}, color.r, color.g, color.b);
            }
        }

        // draw shadow
        for (auto& position : currentShapeShadow.get_absolute_block_positions()) {
            draw_solid_square(&window.bb, {float((position.x + 1) * scale), float((position.y + 1) * scale), scale, scale}, currentShapeShadow.color.r, currentShapeShadow.color.g, currentShapeShadow.color.b, 0xff / 2);
        }

        // draw current shape
        for (auto& position : currentShape.get_absolute_block_positions()) {
            draw_solid_square(&window.bb, {float((position.x + 1) * scale), float((position.y + 1) * scale), scale, scale}, currentShape.color.r, currentShape.color.g, currentShape.color.b);
        }

        sdl_swap_buffer();
    }
}

auto main(int argc, char** argv) -> int {
    if (argc || argv) {}
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
    window.dim.w = baseWindowWidth * scale;
    window.dim.h = baseWindowHeight * scale;

    window.handle = SDL_CreateWindow("Tetris",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               window.dim.w, window.dim.h, SDL_WINDOW_SHOWN);
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

    run();

    return 0;
}
