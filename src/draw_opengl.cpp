#include "platform.hpp"
#include "draw_opengl.hpp"
#include "core.hpp"

auto draw_opengl(ProgramState& programState, GameState& gameState) -> void {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
