cmake_minimum_required(VERSION 3.5.1)

add_compile_options(-Wall -Wextra -Werror -Wfatal-errors -Wno-unused-parameter)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++20")

project(serial)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/config
)

file(GLOB_RECURSE SERIAL_TEST LIST_DIRECTORIES false
  test.cpp
)

add_compile_options(-Wno-implicit-fallthrough)

add_executable(test ${SERIAL_TEST})
