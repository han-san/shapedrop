cmake_minimum_required(VERSION 3.0)

project(ShapeDrop)

add_executable(ShapeDrop src/draw_software.cpp src/draw_opengl.cpp src/platform/sdlmain.cpp src/font.cpp src/board.cpp src/core.cpp src/draw.cpp src/shape.cpp src/tests.cpp src/ui.cpp src/input.cpp src/simulate.cpp)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

add_subdirectory("deps/glad")

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)
find_package(fmt REQUIRED)
find_package(Microsoft.GSL REQUIRED)

if (OpenGL_EGL_FOUND)
    target_include_directories(ShapeDrop PUBLIC ${OPENGL_EGL_INCLUDE_DIR})
    target_link_libraries(ShapeDrop PUBLIC OpenGL::EGL)
else()
    target_include_directories(ShapeDrop PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(ShapeDrop PUBLIC OpenGL::GL)
endif()

target_compile_features(ShapeDrop PRIVATE cxx_std_20)

target_compile_options(ShapeDrop PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->)

target_link_libraries(ShapeDrop PUBLIC
    SDL2::Main
    fmt::fmt
    glad
    glm::glm
    Microsoft.GSL::GSL
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res/font/DejaVuSans.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
