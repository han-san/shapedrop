cmake_minimum_required(VERSION 3.0)

project(ShapeDrop)

add_executable(ShapeDrop src/draw_software.cpp src/draw_opengl.cpp src/platform/sdlmain.cpp src/font.cpp src/board.cpp src/core.cpp src/draw.cpp src/shape.cpp src/tests.cpp src/ui.cpp src/input.cpp src/simulate.cpp)

add_subdirectory("deps/SDL2-2.0.12")
add_subdirectory("deps/fmt-7.0.3")
add_subdirectory("deps/glad")
add_subdirectory("deps/glm-0.9.9.8")

find_package(OpenGL REQUIRED)

if (OpenGL_EGL_FOUND)
    target_include_directories(ShapeDrop PUBLIC ${OPENGL_EGL_INCLUDE_DIR})
    target_link_libraries(ShapeDrop PUBLIC OpenGL::EGL)
else()
    target_include_directories(ShapeDrop PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(ShapeDrop PUBLIC OpenGL::GL)
endif()

target_compile_features(ShapeDrop PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ShapeDrop PRIVATE /W4)
    target_link_libraries(ShapeDrop PUBLIC SDL2main)
else()
    target_compile_options(ShapeDrop PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(ShapeDrop PUBLIC
    SDL2-static
    fmt::fmt
    glad
    glm
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res/font/DejaVuSans.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
