#include "platform.hpp"
#include "draw_opengl.hpp"
#include "core.hpp"
#include "draw.hpp"

#include "glad/glad.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <utility>

namespace OpenGLRender {

Shader::Shader(GLenum shaderType, GLchar const* src) {
    auto shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &src, nullptr);
    glCompileShader(shaderHandle);
    {
        GLint success = 0;
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
        if (success == 0) {
            char infoLog[512];
            glGetShaderInfoLog(shaderHandle, 512, nullptr, infoLog);
            throw std::exception(infoLog);
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
            char infoLog[512];
            glGetProgramInfoLog(programHandle, 512, nullptr, infoLog);
            throw std::exception(infoLog);
        }
    }

    m_handle = programHandle;
}

Context::Context() {
    // Set up the solid shader's vbo, vao, and ebo.
    {
        GLfloat vertices[] = {
            0.F, 1.F, 0.F, // top left
            1.F, 1.F, 0.F, // top right
            0.F, 0.F, 0.F, // bottom left
            1.F, 0.F, 0.F, // bottom right
        };

        GLuint indices[] = {
            0, 1, 3,
            0, 2, 3,
        };

        glGenBuffers(1, &m_solidShaderVBO);
        glGenVertexArrays(1, &m_solidShaderVAO);
        glGenBuffers(1, &m_solidShaderEBO);

        glBindVertexArray(m_solidShaderVAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_solidShaderVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_solidShaderEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // provide 3 floats to vertex shader??
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    }

    // Set up the rainbow shader's vbo, vao, and ebo.
    {
        GLfloat vertices[] = {
            // Positions   // Colors
            -1.F,  1.F, 0.F, 0.F, 1.F, 0.F, // top left
             1.F,  1.F, 0.F, 1.F, 1.F, 0.F, // top right
            -1.F, -1.F, 0.F, 0.F, 1.F, 1.F, // bottom left
             1.F, -1.F, 0.F, 1.F, 0.F, 1.F, // bottom right
        };

        GLuint indices[] = {
            0, 1, 3,
            0, 2, 3,
        };

        glGenBuffers(1, &m_rainbowShaderVBO);
        glGenVertexArrays(1, &m_rainbowShaderVAO);
        glGenBuffers(1, &m_rainbowShaderEBO);

        glBindVertexArray(m_rainbowShaderVAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_rainbowShaderVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rainbowShaderEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // provide 3 floats to vertex shader??
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    }

    // Set up the font shader's vbo, vao, ebo, and textures
    {
        auto const& bakedCharsBitmap = get_baked_chars_bitmap();

        glGenTextures(1, &m_fontTexture);
        glBindTexture(GL_TEXTURE_2D, m_fontTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, bakedCharsBitmap.w, bakedCharsBitmap.h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bakedCharsBitmap.bitmap.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glGenVertexArrays(1, &m_fontShaderVAO);
        glGenBuffers(1, &m_fontShaderVBO);

        glBindVertexArray(m_fontShaderVAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_fontShaderVBO);
        // the vertex- and texture data gets filled in every time a character is drawn,
        // so there's no need for them here.
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

Context::Context(Context&& other) noexcept
    : m_solid {std::move(other.m_solid)}
    , m_rainbow {std::move(other.m_rainbow)}
{
    delete_solid_shader_buffers();
    m_solidShaderEBO = std::exchange(other.m_solidShaderEBO, 0);
    m_solidShaderVAO = std::exchange(other.m_solidShaderVAO, 0);
    m_solidShaderVBO = std::exchange(other.m_solidShaderVBO, 0);

    delete_rainbow_shader_buffers();
    m_rainbowShaderEBO = std::exchange(other.m_rainbowShaderEBO, 0);
    m_rainbowShaderVAO = std::exchange(other.m_rainbowShaderVAO, 0);
    m_rainbowShaderVBO = std::exchange(other.m_rainbowShaderVBO, 0);

    delete_font_shader_buffers();
    m_fontShaderVAO = std::exchange(other.m_fontShaderVAO, 0);
    m_fontShaderVBO = std::exchange(other.m_fontShaderVBO, 0);
}

auto Context::operator =(Context&& other) noexcept -> Context& {
    if (this == &other) {
        return *this;
    }

    delete_solid_shader_buffers();
    m_solid = std::move(other.m_solid);
    m_solidShaderEBO = std::exchange(other.m_solidShaderEBO, 0);
    m_solidShaderVAO = std::exchange(other.m_solidShaderVAO, 0);
    m_solidShaderVBO = std::exchange(other.m_solidShaderVBO, 0);

    delete_rainbow_shader_buffers();
    m_rainbow = std::move(other.m_rainbow);
    m_rainbowShaderEBO = std::exchange(other.m_rainbowShaderEBO, 0);
    m_rainbowShaderVAO = std::exchange(other.m_rainbowShaderVAO, 0);
    m_rainbowShaderVBO = std::exchange(other.m_rainbowShaderVBO, 0);

    delete_font_shader_buffers();
    m_fontShaderVAO = std::exchange(other.m_fontShaderVAO, 0);
    m_fontShaderVBO = std::exchange(other.m_fontShaderVBO, 0);

    return *this;
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

    // draw rainbow background
    {
        get_opengl_render_context().rainbow_shader().use();
        glBindVertexArray(get_opengl_render_context().rainbow_shader_vao());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    switch (programState.levelType) {
    case ProgramState::LevelType::Menu: {
    } break;
    case ProgramState::LevelType::Game: {
        // draw playarea
        {
            auto const& solidShader = get_opengl_render_context().solid_shader();
            solidShader.use();
            solidShader.set_vec4(Shader::Uniform::color, GLColor {Color::black});

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
                solidShader.set_matrix4(Shader::Uniform::model, model);
            }

            solidShader.set_matrix4(Shader::Uniform::projection, orthoProjection);

            glBindVertexArray(get_opengl_render_context().solid_shader_vao());
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }

        auto draw_shape_in_play_area = [](Shape const& shape) {
            auto const scale = get_window_scale();
            for (auto const& position : shape.get_absolute_block_positions()) {
                // since the top 2 rows shouldn't be visible, the y
                // position for drawing is 2 less than the shape's
                auto const actualYPosition = position.y - 2;

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

        for (std::size_t y = 2; y < Board::rows; ++y) {
            for (std::size_t x = 0; x < Board::columns; ++x) {
                auto const currIndex = y * Board::columns + x;
                auto const& block = gameState.board.data[currIndex];
                if (!block.isActive) {
                    continue;
                }

                auto const scale = static_cast<std::size_t>(get_window_scale());

                Rect<int> square {
                    static_cast<int>((x + gPlayAreaDim.x) * scale),
                    static_cast<int>((y - 2 + gPlayAreaDim.y) * scale),
                    static_cast<int>(scale),
                    static_cast<int>(scale)
                };
                draw_solid_square(square, block.color);
            }
        }

        draw_shape_in_play_area(gameState.currentShape);
        // FIXME: the shadow doesn't seem to be transparent?
        draw_shape_in_play_area(gameState.currentShapeShadow);

        // draw shape previews
        {
            auto const scale = get_window_scale();
            auto const previewArray = gameState.shapePool.get_preview_shapes_array();
            int i {0};
            for (auto const shapeType : previewArray) {
                Shape shape {shapeType};
                shape.pos.x = gSidebarDim.x;
                int const ySpacing {3};
                shape.pos.y = gSidebarDim.y + ySpacing * i;
                for (auto const& position : shape.get_absolute_block_positions()) {
                    Rect<int> square {
                        position.x * scale,
                        position.y * scale,
                        scale,
                        scale
                    };
                    draw_solid_square(square, shape.color);
                }
                ++i;
            }
        }

        // draw held shape
        {
            auto const scale = get_window_scale();
            auto const holdShapeDim = gHoldShapeDim * scale;
            draw_solid_square(holdShapeDim, Color::black);
            if (gameState.holdShapeType) {
                Shape shape {*gameState.holdShapeType};
                shape.pos = {};

                auto is_even = [](auto const n) { return (n % 2) == 0; };
                // offset to center shape inside hold square
                auto const shapeDimensions = shape.dimensions();
                auto const xOffset = is_even(gHoldShapeDim.w - shapeDimensions.w) ? 1.0 : 0.5;
                auto const yOffset = is_even(gHoldShapeDim.h - shapeDimensions.h) ? 0.0 : 0.5;

                for (auto& position : shape.get_absolute_block_positions()) {
                    Rect<int> square {
                        static_cast<int>((position.x + gHoldShapeDim.x + xOffset) * scale),
                        static_cast<int>((position.y + gHoldShapeDim.y + yOffset) * scale),
                        scale,
                        scale
                    };
                    draw_solid_square(square, shape.color);
                }
            }
        }
    } break;
    }

    for (auto object : drawObjects) {
        object.shaderProgram.use();
        object.shaderProgram.set_vec4(Shader::Uniform::color, GLColor {object.color});

        {
            glm::mat4 model {1};
            model = glm::translate(model, glm::vec3 {object.rect.x, object.rect.y, 0.F});
            model = glm::scale(model, glm::vec3 {object.rect.w, object.rect.h, 0.F});
            object.shaderProgram.set_matrix4(Shader::Uniform::model, model);
        }

        object.shaderProgram.set_matrix4(Shader::Uniform::projection, orthoProjection);

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
