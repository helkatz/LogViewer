#cmake_minimum_required(VERSION 3.0)

project(common_widgets)

message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources src/* include/*)

message(${Qt5Widgets_LIBRARIES})
#set(CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/plugins)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DQDESIGNER_EXPORT_WIDGETS)

add_library(${PROJECT_NAME} SHARED ${sources})

SOURCE_GROUP_BY_FOLDER(sources)

message(${PROJECT_SOURCE_DIR})
message(${CMAKE_SOURCE_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC include)


target_link_libraries(
    ${PROJECT_NAME} 
		Qt5::Widgets Qt5::Sql
    )
conan_target_link_libraries(${PROJECT_NAME})

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
message("QT_PLUGINS_DIR ${QT_PLUGINS_DIR}")
install(TARGETS ${PROJECT_NAME}
    DESTINATION ${QT_PLUGINS_DIR}/designer
)