#cmake_minimum_required(VERSION 3.0)

project(tests)

message(" - tests")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests)

add_subdirectory(core)
