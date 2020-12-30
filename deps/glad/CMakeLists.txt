cmake_minimum_required(VERSION 3.0)

project(GLAD)

set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(GLAD_INCLUDE_DIRS "${GLAD_DIR}/include")
list(APPEND GLAD_SOURCES "${GLAD_DIR}/src/glad.c")

add_library(glad ${GLAD_SOURCES})
target_include_directories(glad PUBLIC $<BUILD_INTERFACE:${GLAD_INCLUDE_DIRS}>)
