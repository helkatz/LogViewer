#cmake_minimum_required(VERSION 3.0)
project(core_test)
message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources src/* include/*)

add_executable(${PROJECT_NAME} WIN32 ${sources})
SOURCE_GROUP_BY_FOLDER(sources)

target_link_libraries(${PROJECT_NAME} 
	core
	#CONAN_PKG::boost
	#CONAN_PKG::gtest
	)
conan_target_link_libraries(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC include )
set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER LogViewer)
#message(${CONAN_LIBS})
