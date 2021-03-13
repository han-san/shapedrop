#pragma once

#include "core.hpp"
#include "font.hpp"
#include "util.hpp"

#include "glad/glad.h"

#include <string_view>

namespace OpenGLRender {

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
private:
    GLuint m_handle {0};
};

class ShaderProgram {
public:
    ShaderProgram(GLchar const* vertexSource, GLchar const* fragmentSource);
    ShaderProgram(ShaderProgram const&) = delete;
    auto operator =(ShaderProgram const&) = delete;
    ShaderProgram(ShaderProgram&& other) noexcept {
        glDeleteProgram(m_handle);
        m_handle = other.m_handle;
        other.m_handle = 0;
    }
    auto operator =(ShaderProgram&& other) noexcept -> ShaderProgram& {
        glDeleteProgram(m_handle);
        m_handle = other.m_handle;
        other.m_handle = 0;
        return *this;
    }

    ~ShaderProgram() {
        glDeleteProgram(m_handle);
    }

    [[nodiscard]]
    auto handle() const -> GLuint {
        return m_handle;
    }

    auto use() const -> void {
        glUseProgram(m_handle);
    }

private:
    GLuint m_handle {0};
};

class Context {
public:
    [[nodiscard]]
    auto solid_shader() const -> ShaderProgram const& {
        return m_solid;
    }

private:
    ShaderProgram m_solid {
        R"foo(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
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
};


auto draw(ProgramState& programState, GameState& gameState) -> void;

auto draw_solid_square_normalized(Rect<double> sqr, Color::RGBA color) -> void;
auto draw_solid_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color) -> void;
auto draw_hollow_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color, int borderSize = 1) -> void;
auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr, Color::RGBA color, int borderSize = 1) -> void;
auto draw_font_string(BackBuffer& buf, FontString const& fontString, Point<int> coords) -> void;
auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString, Point<double> relativeCoords) -> void;
auto draw_text(BackBuffer& buf, std::string_view text, Point<int> coords, double pixelHeight) -> void;
auto draw_text_normalized(BackBuffer& buf, std::string_view text, Point<double> relativeCoords, double pixelHeight) -> void;

}
