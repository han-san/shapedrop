cmake_minimum_required(VERSION 2.8.10)

project(ShapeDrop)

add_executable(ShapeDrop src/platform/sdlmain.cpp src/board.cpp src/core.cpp src/draw.cpp src/shape.cpp src/tests.cpp)

set(SDL2_DIR "deps/SDL2-2.0.10")
find_package(SDL2 REQUIRED)

target_include_directories(ShapeDrop PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${SDL2_INCLUDE_DIR})
target_compile_features(ShapeDrop PRIVATE cxx_std_17)

target_link_libraries(ShapeDrop ${SDL2_LIBRARIES})

install(TARGETS ShapeDrop RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin)
install(FILES ${SDL2_DIR}/lib/x86/SDL2.dll DESTINATION ${PROJECT_SOURCE_DIR}/bin)
