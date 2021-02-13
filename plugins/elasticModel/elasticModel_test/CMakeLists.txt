#cmake_minimum_required(VERSION 3.0)
project(elasticModel_test)
message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources src/* include/*)

add_executable(${PROJECT_NAME} ${sources})
SOURCE_GROUP_BY_FOLDER(sources)

target_link_libraries(${PROJECT_NAME} core elasticModel)
conan_target_link_libraries(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC include ../elasticModel/src)
set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins/elasticModel)
