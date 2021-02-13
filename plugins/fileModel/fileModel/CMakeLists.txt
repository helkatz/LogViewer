#cmake_minimum_required(VERSION 3.0)

project(fileModel)

message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources * gui/* include/*)

set(CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/plugins)

add_library(${PROJECT_NAME} SHARED ${sources})

SOURCE_GROUP_BY_FOLDER(sources)

message(${PROJECT_SOURCE_DIR})
message(${CMAKE_SOURCE_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC include gui/include)


target_link_libraries(
    ${PROJECT_NAME} 
		core
		Qt5::Widgets Qt5::Sql
    )
conan_target_link_libraries(${PROJECT_NAME})

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins/fileModel)