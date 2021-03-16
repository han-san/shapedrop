#include "platform.hpp"
#include "draw_opengl.hpp"
#include "core.hpp"
#include "draw.hpp"

#include "glad/glad.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace OpenGLRender {

Shader::Shader(GLenum shaderType, GLchar const* src) {
    auto shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &src, nullptr);
    glCompileShader(shaderHandle);
    {
        GLint success = 0;
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
        if (success == 0) {
            throw;
        }
    }
    m_handle = shaderHandle;
}

Shader::Program::Program(GLchar const* vertexSource, GLchar const* fragmentSource) {
    Shader vertex {GL_VERTEX_SHADER, vertexSource};
    Shader fragment {GL_FRAGMENT_SHADER, fragmentSource};

    auto programHandle = glCreateProgram();
    glAttachShader(programHandle, vertex.handle());
    glAttachShader(programHandle, fragment.handle());
    glLinkProgram(programHandle);
    {
        GLint success = 0;
        glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
        if (success == 0) {
            throw;
        }
    }

    m_handle = programHandle;
}

struct DrawObject {
    GLuint vao;
    Shader::Program const& shaderProgram;
    Color::RGBA color;
    Rect<double> rect;
};

std::vector<DrawObject> drawObjects;

auto draw(ProgramState& programState, GameState& gameState) -> void {
    glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);

    // The y axis is flipped, i.e. starts at 1.F and ends at 0.F.
    auto const orthoProjection = glm::ortho(0.F, 1.F, 1.F, 0.F);

    switch (programState.levelType) {
    case ProgramState::LevelType::Menu: {
    } break;
    case ProgramState::LevelType::Game: {
        // draw playarea
        {
            auto const& solidShader = get_opengl_render_context().solid_shader();
            solidShader.use();
            solidShader.set_vec4("color", GLColor {Color::black});

            {
                glm::mat4 model {1};
                auto const scale = get_window_scale();
                auto const normalizedPlayArea = to_normalized({
                                                            static_cast<double>(gPlayAreaDim.x) * scale,
                                                            static_cast<double>(gPlayAreaDim.y) * scale,
                                                            static_cast<double>(gPlayAreaDim.w) * scale,
                                                            static_cast<double>(gPlayAreaDim.h) * scale,
                                                            });
                model = glm::translate(model, glm::vec3 {normalizedPlayArea.x, normalizedPlayArea.y, 0.F});
                model = glm::scale(model, glm::vec3 {normalizedPlayArea.w, normalizedPlayArea.h, 0.F});
                solidShader.set_matrix4("model", model);
            }

            solidShader.set_matrix4("projection", orthoProjection);

            glBindVertexArray(get_opengl_render_context().solid_shader_vao());
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }

        auto draw_shape_in_play_area = [](Shape const& shape) {
            auto const scale = get_window_scale();
            for (auto const& position : shape.get_absolute_block_positions()) {
                // since the top 2 rows shouldn't be visible, the y
                // position for drawing is 2 less than the shape's
                auto const actualYPosition {position.y - 2};

                // don't draw if square is above the playarea
                if (actualYPosition + gPlayAreaDim.y < gPlayAreaDim.y) { continue; }
                Rect<int> square {
                    (position.x + gPlayAreaDim.x) * scale,
                    (actualYPosition + gPlayAreaDim.y) * scale,
                    scale,
                    scale
                };

                draw_solid_square(square, shape.color);
            }
        };

        draw_shape_in_play_area(gameState.currentShape);
        // FIXME: the shadow doesn't seem to be transparent?
        draw_shape_in_play_area(gameState.currentShapeShadow);
    } break;
    }

    for (auto object : drawObjects) {
        object.shaderProgram.use();
        object.shaderProgram.set_vec4("color", GLColor {object.color});

        {
            glm::mat4 model {1};
            model = glm::translate(model, glm::vec3 {object.rect.x, object.rect.y, 0.F});
            model = glm::scale(model, glm::vec3 {object.rect.w, object.rect.h, 0.F});
            object.shaderProgram.set_matrix4("model", model);
        }

        object.shaderProgram.set_matrix4("projection", orthoProjection);

        glBindVertexArray(object.vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);

    drawObjects.clear();
}

auto draw_solid_square_normalized(Rect<double> sqr, Color::RGBA color) -> void {
    // FIXME: probably need to flip the image upside down since opengl counts
    //        0,0 as bottom left corner while we count 0,0 as top left.

    auto const& renderContext = get_opengl_render_context();
    auto const& shaderProgram = renderContext.solid_shader();

    drawObjects.push_back({renderContext.solid_shader_vao(), shaderProgram, color, sqr});
}

auto draw_solid_square(Rect<int> sqr, Color::RGBA color) -> void {
    auto const& renderContext = get_opengl_render_context();
    auto normalized = to_normalized(
                                    {
                                    static_cast<double>(sqr.x),
                                    static_cast<double>(sqr.y),
                                    static_cast<double>(sqr.w),
                                    static_cast<double>(sqr.h),
                                    }
                                   );
    draw_solid_square_normalized(normalized, color);
}
auto draw_hollow_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color, int borderSize) -> void {
}
auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color, int borderSize) -> void {
}
auto draw_font_string(BackBuffer& buf, FontString const& fontString, Point<int> coords) -> void {
}
auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString, Point<double> relativeCoords) -> void {
}
auto draw_text(BackBuffer& buf, std::string_view text, Point<int> coords, double pixelHeight) -> void {
}
auto draw_text_normalized(BackBuffer& buf, std::string_view text, Point<double> relativeCoords, double pixelHeight) -> void {
}

}
