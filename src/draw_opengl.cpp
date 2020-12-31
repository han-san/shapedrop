#include "platform.hpp"
#include "draw_opengl.hpp"
#include "core.hpp"
#include "draw.hpp"

#include "glad/glad.h"

namespace OpenGLRender {

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

auto static create_solid_color_shader() {
    char const* vertexShaderSource = R"foo(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
        )foo";

    auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    {
        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (success == 0) {
            throw;
        }
    }

    char const* fragmentShaderSource = R"foo(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 color;
        void main() {
            FragColor = color;
        }
        )foo";

    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    {
        GLint success;
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (success == 0) {
            throw;
        }
    }

    auto shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    {
        GLint success;
        glGetProgramiv(fragmentShader, GL_LINK_STATUS, &success);
        if (success == 0) {
            throw;
        }
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    static auto shaderProgram = create_solid_color_shader();

    drawObjects.push_back({vao, shaderProgram, color});
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