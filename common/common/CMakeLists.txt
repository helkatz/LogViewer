#cmake_minimum_required(VERSION 3.0)

project(common)

message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources src/* include/*)

add_library(${PROJECT_NAME} STATIC ${sources})

SOURCE_GROUP_BY_FOLDER(sources)

message(${PROJECT_SOURCE_DIR})
message(${CMAKE_SOURCE_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC include )

target_link_libraries(
    ${PROJECT_NAME} 
    )
conan_target_link_libraries(${PROJECT_NAME})

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER common)