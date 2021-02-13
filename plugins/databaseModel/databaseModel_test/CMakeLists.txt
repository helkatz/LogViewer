#cmake_minimum_required(VERSION 3.0)
get_filename_component(PARENT_DIR ${PROJECT_SOURCE_DIR} DIRECTORY)
message("parent folder ${PARENT_DIR}")
project(databaseModel_test)
message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources src/* include/*)

add_executable(${PROJECT_NAME} ${sources})
SOURCE_GROUP_BY_FOLDER(sources)


target_link_libraries(${PROJECT_NAME} databaseModel)
conan_target_link_libraries(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC include ../databaseModel/src)
set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins/databaseModel)
