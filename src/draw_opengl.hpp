#pragma once

#include "core.hpp"
#include "font.hpp"
#include "util.hpp"

#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"

#include <utility>

namespace OpenGLRender {

struct GLColor {
  float r;
  float g;
  float b;
  float a;

  // OpenGL uses 0 -> 1 while we use 0 -> maxChannelValue
  explicit constexpr GLColor(Color::RGBA color)
      : r {static_cast<float>(u8 {color.r}) / Color::RGBA::maxChannelValue},
        g {static_cast<float>(u8 {color.g}) / Color::RGBA::maxChannelValue},
        b {static_cast<float>(u8 {color.b}) / Color::RGBA::maxChannelValue},
        a {static_cast<float>(u8 {color.a}) / Color::RGBA::maxChannelValue} {}
};

class Shader {
public:
  Shader(GLenum shaderType, GLchar const* src);
  Shader(Shader const&) = delete;
  auto operator=(Shader const&) = delete;
  Shader(Shader&& other) noexcept {
    glDeleteShader(m_handle);
    m_handle = std::exchange(other.m_handle, 0);
  }
  auto operator=(Shader&& other) noexcept -> Shader& {
    if (this == &other) {
      return *this;
    }

    glDeleteShader(m_handle);
    m_handle = std::exchange(other.m_handle, 0);
    return *this;
  }
  ~Shader() { glDeleteShader(m_handle); }

  [[nodiscard]] auto handle() const noexcept { return m_handle; }

  enum class Uniform {
    color,
    model,
    projection,
  };

  auto static constexpr to_string_view(Uniform const u) -> std::string_view {
    switch (u) {
    case Uniform::color:
      return "color";
    case Uniform::model:
      return "model";
    case Uniform::projection:
      return "projection";
    }
  }

  auto static to_string(Uniform const u) -> std::string {
    return std::string(to_string_view(u));
  }

  class Program {
  public:
    Program(GLchar const* vertexSource, GLchar const* fragmentSource);
    Program(Program const&) = delete;
    auto operator=(Program const&) = delete;
    Program(Program&& other) noexcept {
      glDeleteProgram(m_handle);
      m_handle = std::exchange(other.m_handle, 0);
    }
    auto operator=(Program&& other) noexcept -> Program& {
      if (this == &other) {
        return *this;
      }

      glDeleteProgram(m_handle);
      m_handle = std::exchange(other.m_handle, 0);
      return *this;
    }

    ~Program() { glDeleteProgram(m_handle); }

    [[nodiscard]] auto handle() const -> GLuint { return m_handle; }

    auto use() const -> void { glUseProgram(m_handle); }

    auto set_matrix4(Uniform u, glm::mat4 const& mat) const -> void {
      auto const name = to_string_view(u);
      auto const uniformLoc = glGetUniformLocation(m_handle, name.data());
      glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
    }

    auto set_vec4(Uniform u, float x, float y, float z, float w) const -> void {
      auto const name = to_string_view(u);
      auto uniformLoc = glGetUniformLocation(m_handle, name.data());
      glUniform4f(uniformLoc, x, y, z, w);
    }

    auto set_vec4(Uniform u, GLColor const color) const -> void {
      auto const name = to_string_view(u);
      auto uniformLoc = glGetUniformLocation(m_handle, name.data());
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
  Context();
  Context(Context const&) = delete;
  auto operator=(Context const&) = delete;
  Context(Context&& other) noexcept;
  auto operator=(Context&& other) noexcept -> Context&;
  ~Context() noexcept {
    delete_solid_shader_buffers();
    delete_rainbow_shader_buffers();
    delete_font_shader_buffers();
  }

  [[nodiscard]] auto solid_shader() const -> Shader::Program const& {
    return m_solid;
  }
  [[nodiscard]] auto solid_shader_vao() const -> GLuint {
    return m_solidShaderVAO;
  }

  [[nodiscard]] auto rainbow_shader() const -> Shader::Program const& {
    return m_rainbow;
  }
  [[nodiscard]] auto rainbow_shader_vao() const -> GLuint {
    return m_rainbowShaderVAO;
  }

  [[nodiscard]] auto font_shader_vao() const -> GLuint {
    return m_fontShaderVAO;
  }

private:
  auto delete_solid_shader_buffers() -> void {
    glDeleteBuffers(1, &m_solidShaderEBO);
    glDeleteBuffers(1, &m_solidShaderVBO);
    glDeleteVertexArrays(1, &m_solidShaderVAO);
  }
  auto delete_rainbow_shader_buffers() -> void {
    glDeleteBuffers(1, &m_rainbowShaderEBO);
    glDeleteBuffers(1, &m_rainbowShaderVBO);
    glDeleteVertexArrays(1, &m_rainbowShaderVAO);
  }

  auto delete_font_shader_buffers() -> void {
    glDeleteBuffers(1, &m_fontShaderVBO);
    glDeleteVertexArrays(1, &m_fontShaderVAO);
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
        )foo"};

  Shader::Program m_rainbow {
      R"foo(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;

        out vec3 color;

        void main() {
            gl_Position = vec4(aPos, 1.0);
            color = aColor;
        }
        )foo",
      R"foo(
        #version 330 core
        out vec4 FragColor;
        in vec3 color;

        void main() {
            FragColor = vec4(color, 1.0);
        }
        )foo"};

  GLuint m_solidShaderVAO {0};
  GLuint m_solidShaderVBO {0};
  GLuint m_solidShaderEBO {0};

  GLuint m_rainbowShaderVAO {0};
  GLuint m_rainbowShaderVBO {0};
  GLuint m_rainbowShaderEBO {0};

  GLuint m_fontShaderVAO {0};
  GLuint m_fontShaderVBO {0};
  GLuint m_fontTexture {0};
};

auto draw(ProgramState& programState, GameState& gameState) -> void;

auto draw_solid_square_normalized(Rect<double> sqr, Color::RGBA color) -> void;
auto draw_solid_square(Rect<int> sqr, Color::RGBA color) -> void;
auto draw_hollow_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color,
                        int borderSize = 1) -> void;
auto draw_hollow_square_normalized(BackBuffer& buf, Rect<double> sqr,
                                   Color::RGBA color, int borderSize = 1)
    -> void;
auto draw_font_string(BackBuffer& buf, FontString const& fontString,
                      Point<int> coords) -> void;
auto draw_font_string_normalized(BackBuffer& buf, FontString const& fontString,
                                 Point<double> relativeCoords) -> void;
auto draw_text(BackBuffer& buf, std::string_view text, Point<int> coords,
               double pixelHeight) -> void;
auto draw_text_normalized(BackBuffer& buf, std::string_view text,
                          Point<double> relativeCoords, double pixelHeight)
    -> void;

} // namespace OpenGLRender
