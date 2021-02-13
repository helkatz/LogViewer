#cmake_minimum_required(VERSION 3.0)

project(main)

message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources * gui/* ../assets/*.qrc)

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)
find_package(Qt5Widgets)

# qt5_add_resources(RCC_SOURCES default.qrc)
add_executable(main ${sources})

SOURCE_GROUP_BY_FOLDER(sources)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(
    ${PROJECT_NAME} 
	core
	Qt5::Widgets
    )
#conan_target_link_libraries(${PROJECT_NAME})

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER LogViewer)

IF(WIN32) # Check if we are on Windows
  if(MSVC) # Check if we are using the Visual Studio compiler
    set_target_properties(${PROJECT_NAME} PROPERTIES 
		LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS"
		LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") # Not tested
  else()
    message(SEND_ERROR "You are using an unsupported Windows compiler! (Not MSVC or GCC)")
  endif()
elseif(UNIX)
  # Nothing special required
else()
  message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()