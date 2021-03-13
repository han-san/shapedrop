#include "platform.hpp"
#include "draw_opengl.hpp"
#include "core.hpp"
#include "draw.hpp"

#include "glad/glad.h"

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

ShaderProgram::ShaderProgram(GLchar const* vertexSource, GLchar const* fragmentSource) {
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
    GLuint shaderProgram;
    Color::RGBA color;
};

std::vector<DrawObject> drawObjects;

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

auto draw(ProgramState& programState, GameState& gameState) -> void {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto object : drawObjects) {
        glUseProgram(object.shaderProgram);
        auto vertexColorLocation = glGetUniformLocation(object.shaderProgram, "color");
        GLColor color {object.color};
        glUniform4f(vertexColorLocation, color.r, color.g, color.b, color.a);

        glBindVertexArray(object.vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);

    drawObjects.clear();
}

auto draw_solid_square_normalized(Rect<double> sqr, Color::RGBA color) -> void {
    // FIXME: probably need to flip the image upside down since opengl counts
    //        0,0 as bottom left corner while we count 0,0 as top left.

    // convert between 0 -> 1 normalized to -1 -> 1 normalized
    sqr *= 2;
    sqr.x -= 1;
    sqr.y -= 1;

    float vertices[] = {
        float(sqr.x), float(sqr.y), 0.F, // top left
        float(sqr.x + sqr.w), float(sqr.y), 0.F, // top right
        float(sqr.x), float(sqr.y + sqr.h), 0.F, // bottom left
        float(sqr.x + sqr.w), float(sqr.y + sqr.h), 0.F // bottom right
    };

    unsigned indices[] = {
        0, 1, 3,
        0, 2, 3,
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLuint vao;
    glGenVertexArrays(1, &vao);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // provide 3 floats to vertex shader??
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    auto const& renderContext = get_opengl_render_context();
    auto const& shaderProgram = renderContext.solid_shader();

    drawObjects.push_back({vao, shaderProgram.handle(), color});
}

auto draw_solid_square(BackBuffer& buf, Rect<int> sqr, Color::RGBA color) -> void {
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
