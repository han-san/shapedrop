#pragma once

#include "core.hpp"
#include "font.hpp"
#include "util.hpp"

#include "glad/glad.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace OpenGLRender {

struct GLColor {
    float r;
    float g;
    float b;
    float a;

    // OpenGL uses 0 -> 1 while we use 0 -> maxChannelValue
    explicit constexpr GLColor(Color::RGBA color)
    : r {static_cast<float>(u8 {color.r}) / Color::RGBA::maxChannelValue}
    , g {static_cast<float>(u8 {color.g}) / Color::RGBA::maxChannelValue}
    , b {static_cast<float>(u8 {color.b}) / Color::RGBA::maxChannelValue}
    , a {static_cast<float>(u8 {color.a}) / Color::RGBA::maxChannelValue}
    {}
};

class Shader {
public:
    Shader(GLenum shaderType, GLchar const* src);
    Shader(Shader const&) = delete;
    auto operator =(Shader const&) = delete;
    Shader(Shader&& other) noexcept {
        glDeleteShader(m_handle);
        m_handle = other.m_handle;
        other.m_handle = 0;
    }
    auto operator =(Shader&& other) noexcept -> Shader& {
        glDeleteShader(m_handle);
        m_handle = other.m_handle;
        other.m_handle = 0;
        return *this;
    }
    ~Shader() {
        glDeleteShader(m_handle);
    }

    [[nodiscard]]
    auto handle() const noexcept {
        return m_handle;
    }

    class Program {
    public:
        Program(GLchar const* vertexSource, GLchar const* fragmentSource);
        Program(Program const&) = delete;
        auto operator =(Program const&) = delete;
        Program(Program&& other) noexcept {
            glDeleteProgram(m_handle);
            m_handle = other.m_handle;
            other.m_handle = 0;
        }
        auto operator =(Program&& other) noexcept -> Program& {
            glDeleteProgram(m_handle);
            m_handle = other.m_handle;
            other.m_handle = 0;
            return *this;
        }

        ~Program() {
            glDeleteProgram(m_handle);
        }

        [[nodiscard]]
        auto handle() const -> GLuint {
            return m_handle;
        }

        auto use() const -> void {
            glUseProgram(m_handle);
        }

        auto set_matrix4(GLchar const* uniformName, glm::mat4 const& mat) const -> void {
            auto uniformLoc = glGetUniformLocation(m_handle, uniformName);
            glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
        }

        auto set_vec4(GLchar const* uniformName, float x, float y, float z, float w) const -> void {
            auto uniformLoc = glGetUniformLocation(m_handle, uniformName);
            glUniform4f(uniformLoc, x, y, z, w);
        }

        auto set_vec4(GLchar const* uniformName, GLColor const color) const -> void {
            auto uniformLoc = glGetUniformLocation(m_handle, uniformName);
            auto const [r, g, b, a] = color;
            glUniform4f(uniformLoc, r, g, b, a);
        }

    private:
        GLuint m_handle {0};
    };

private:
    GLuint m_handle {0};
};

class Context {
public:
    Context() {
        // Set up the solid shader's vbo, vao, and ebo.
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
    Context(Context const&) = delete;
    auto operator =(Context const&) = delete;
    Context(Context&& other) noexcept
        : m_solid {std::move(other.m_solid)}
    {
        delete_solid_shader_buffers();
        m_solidShaderEBO = other.m_solidShaderEBO;
        m_solidShaderVAO = other.m_solidShaderVAO;
        m_solidShaderVBO = other.m_solidShaderVBO;
        other.m_solidShaderEBO = 0;
        other.m_solidShaderVAO = 0;
        other.m_solidShaderVBO = 0;
    }
    auto operator =(Context&& other) noexcept -> Context& {
        delete_solid_shader_buffers();
        m_solid = std::move(other.m_solid);
        m_solidShaderEBO = other.m_solidShaderEBO;
        m_solidShaderVAO = other.m_solidShaderVAO;
        m_solidShaderVBO = other.m_solidShaderVBO;
        other.m_solidShaderEBO = 0;
        other.m_solidShaderVAO = 0;
        other.m_solidShaderVBO = 0;
        return *this;
    }
    ~Context() noexcept {
        delete_solid_shader_buffers();
    }

    [[nodiscard]]
    auto solid_shader() const -> Shader::Program const& {
        return m_solid;
    }
    [[nodiscard]] auto solid_shader_vao() const -> GLuint {
        return m_solidShaderVAO;
    }

private:
    auto delete_solid_shader_buffers() -> void {
        glDeleteBuffers(1, &m_solidShaderEBO);
        glDeleteBuffers(1, &m_solidShaderVBO);
        glDeleteVertexArrays(1, &m_solidShaderVAO);
    }

    Shader::Program m_solid {
        R"foo(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        uniform mat4 model;
        uniform mat4 projection;

        void main() {
            gl_Position = projection * model * vec4(aPos, 1.0);
        }
        )foo",
        R"foo(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 color;
        void main() {
            FragColor = color;
        }
        )foo"
    };

    GLuint m_solidShaderVAO {0};
    GLuint m_solidShaderVBO {0};
    GLuint m_solidShaderEBO {0};
};


auto draw(ProgramState& programState, GameState& gameState) -> void;

auto draw_solid_square_normalized(Rect<double> sqr, Color::RGBA color) -> void;
auto draw_solid_square(Rect<int> sqr, Color::RGBA color) -> void;
auto draw_hollow_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color, int borderSize = 1) -> void;
auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color, int borderSize = 1) -> void;
auto draw_font_string(BackBuffer& buf, FontString const& fontString, Point<int> coords) -> void;
auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString, Point<double> relativeCoords) -> void;
auto draw_text(BackBuffer& buf, std::string_view text, Point<int> coords, double pixelHeight) -> void;
auto draw_text_normalized(BackBuffer& buf, std::string_view text, Point<double> relativeCoords, double pixelHeight) -> void;

}
