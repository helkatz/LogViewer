cmake_minimum_required(VERSION 2.8)

project(LogViewer)# VERSION 1.2.3.4 LANGUAGES C CXX)
enable_testing()

message(LogViewer)

###########################################################
# conan specific "header"
###########################################################
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/memsharded/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP #UPDATE
				BUILD missing)

###########################################################
# END conan specific "header"
###########################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /MP /Zi /DEBUG")
	SET(CMAKE_LINKER_FLAGS "${CMAKE_CXX_FLAGS} /DEBUG")
else()
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Wall -std=c++1z -static-libstdc++ -Wno-error=unused -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations -Wno-deprecated")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -std=c++1z -static-libstdc++ -Wno-error=unused -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations -Wno-deprecated")
endif()

# Qt 
SET(CMAKE_PREFIX_PATH "/qt/qt5.9.9/5.9.9/msvc2017_64")

cmake_policy(SET CMP0020 NEW)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets)
find_package(Qt5Widgets)
find_package(Qt5Sql)
find_package(Qt5Network)
find_package(Qt5Svg)
qt5_add_resources(RCC_SOURCES default.qrc)
# message(${RCC_SOURCES})
# add_executable(main ${RCC_SOURCES} ${sources})
# Qt End

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

MACRO(SOURCE_GROUP_BY_FOLDER source_files)
  SET(SOURCE_GROUP_DELIMITER "/")
  SET(last_dir "")
  SET(files "")
#  FOREACH(file ${${target}_SRC} ${${target}_HEADERS})
  FOREACH(file ${${source_files}})
    file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}" ${file})
    GET_FILENAME_COMPONENT(dir "${relative_file}" PATH)
    IF (NOT "${dir}" STREQUAL "${last_dir}")
      IF (files)
        SOURCE_GROUP("${last_dir}" FILES ${files})
      ENDIF (files)
      SET(files "")
    ENDIF (NOT "${dir}" STREQUAL "${last_dir}")
    SET(files ${files} ${file})
    SET(last_dir "${dir}")
  ENDFOREACH(file)
  IF (files)
    SOURCE_GROUP("${last_dir}" FILES ${files})
  ENDIF (files)
ENDMACRO(SOURCE_GROUP_BY_FOLDER)

message(${PROJECT_SOURCE_DIR})




#add_subdirectory(external)
#add_subdirectory(../logger ${CMAKE_BINARY_DIR}/logger)
add_subdirectory(common)
# add_subdirectory(external)
add_subdirectory(core)
add_subdirectory(plugins)
#add_subdirectory(comon_widgets)
add_subdirectory(main)


