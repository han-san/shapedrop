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
    Rect<double> rect;
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
    glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto object : drawObjects) {
        glUseProgram(object.shaderProgram);
        auto vertexColorLocation = glGetUniformLocation(object.shaderProgram, "color");
        GLColor color {object.color};
        glUniform4f(vertexColorLocation, color.r, color.g, color.b, color.a);

        glm::mat4 model {1};
        model = glm::translate(model, glm::vec3 {object.rect.x, object.rect.y, 0.F});
        model = glm::scale(model, glm::vec3 {object.rect.w, object.rect.h, 0.F});

        auto vertexModelLocation = glGetUniformLocation(object.shaderProgram, "model");
        glUniformMatrix4fv(vertexModelLocation, 1, GL_FALSE, glm::value_ptr(model));

        auto projection = glm::ortho(0.F, 1.F, 0.F, 1.F, 0.F, 1.F);

        auto vertexProjectionLocation = glGetUniformLocation(object.shaderProgram, "projection");
        glUniformMatrix4fv(vertexProjectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

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

    drawObjects.push_back({renderContext.solid_shader_vao(), shaderProgram.handle(), color, sqr});
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
