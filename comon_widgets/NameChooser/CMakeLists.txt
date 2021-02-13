#cmake_minimum_required(VERSION 3.0)

project(NameChooser)

message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources *)

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)

# find_package(Qt5Widgets)
# find_package(Qt5Sql)
# find_package(Qt5Network)
message(${Qt5Widgets_LIBRARIES})
set(CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_definitions(-DQDESIGNER_EXPORT_WIDGETS)

add_library(${PROJECT_NAME} SHARED ${sources})

SOURCE_GROUP_BY_FOLDER(sources)

message(${PROJECT_SOURCE_DIR})
message(${CMAKE_SOURCE_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC *)


target_link_libraries(
    ${PROJECT_NAME} 
		Qt5::Widgets Qt5::Sql
    )
conan_target_link_libraries(${PROJECT_NAME})

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER common_widgets)
message("QT_PLUGINS_DIR ${QT_PLUGINS_DIR}")
install(TARGETS ${PROJECT_NAME}
    DESTINATION ${QT_PLUGINS_DIR}/designer
)