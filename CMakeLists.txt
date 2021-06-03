cmake_minimum_required(VERSION 3.0)

project(ShapeDrop)

add_executable(ShapeDrop src/draw_software.cpp src/draw_opengl.cpp src/platform/sdlmain.cpp src/font.cpp src/board.cpp src/core.cpp src/draw.cpp src/shape.cpp src/tests.cpp src/ui.cpp src/input.cpp src/simulate.cpp)

add_subdirectory("deps/SDL2-2.0.12")
add_subdirectory("deps/fmt-7.0.3")
add_subdirectory("deps/glad")
add_subdirectory("deps/glm-0.9.9.8")
add_subdirectory("deps/GSL-3.1.0/")

find_package(OpenGL REQUIRED)

if (OpenGL_EGL_FOUND)
    target_include_directories(ShapeDrop PUBLIC ${OPENGL_EGL_INCLUDE_DIR})
    target_link_libraries(ShapeDrop PUBLIC OpenGL::EGL)
else()
    target_include_directories(ShapeDrop PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(ShapeDrop PUBLIC OpenGL::GL)
endif()

target_compile_features(ShapeDrop PRIVATE cxx_std_17)

target_compile_options(ShapeDrop PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->)

target_link_libraries(ShapeDrop PUBLIC
    $<$<PLATFORM_ID:Windows>:SDL2main>
    SDL2-static
    fmt::fmt
    glad
    glm
    Microsoft.GSL::GSL
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res/font/DejaVuSans.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
