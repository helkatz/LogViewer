#cmake_minimum_required(VERSION 3.0)

project(core)

message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources * gui/* interfaces/* utils/* include/*)

add_library(${PROJECT_NAME} SHARED ${sources})

SOURCE_GROUP_BY_FOLDER(sources)

target_include_directories(${PROJECT_NAME} PUBLIC 
	include gui/include utils/include interfaces/include)

target_link_libraries(
    ${PROJECT_NAME} 
		#fileModel elasticModel databaseModel
		Qt5::Widgets Qt5::Sql Qt5::Svg
		common_widgets
		common
    )
conan_target_link_libraries(${PROJECT_NAME})
set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER LogViewer)