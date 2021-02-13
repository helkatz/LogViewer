include("c:/data/binaries/cmake/helper.cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)



message(${PROJECT_SOURCE_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#set(CMAKE_PREFIX_PATH C:/qt/5.9/5.9.1/msvc2015)
#set(CMAKE_PREFIX_PATH C:/qt5/5.9.3/msvc2015)
set(CMAKE_PREFIX_PATH C:/qt5/5.10.0/msvc2015)
#set(CMAKE_PREFIX_PATH C:/qt/5.7/msvc2015)
##file(GLOB_RECURSE ui_files *.ui)
#find_package(Qt5)

find_package(Qt5Widgets)
find_package(Qt5Sql)
find_package(Qt5Network)

if(Qt5Widgets_LIBRARIES)
	add_library(Qt5Widgets STATIC IMPORTED GLOBAL)

	set_target_properties(Qt5Widgets PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${Qt5Widgets_INCLUDE_DIRS}"
		INTERFACE_LINK_LIBRARIES "${Qt5Widgets_LIBRARIES}"
	)
endif()

if(Qt5Sql_LIBRARIES)	
	add_library(Qt5Sql STATIC IMPORTED GLOBAL)

	set_target_properties(Qt5Sql PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${Qt5Sql_INCLUDE_DIRS}"
		INTERFACE_LINK_LIBRARIES "${Qt5Sql_LIBRARIES}"
	)
endif()
if(Qt5Network_LIBRARIES)	
	add_library(Qt5Network STATIC IMPORTED GLOBAL)

	set_target_properties(Qt5Network PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${Qt5Network_INCLUDE_DIRS}"
		INTERFACE_LINK_LIBRARIES "${Qt5Network_LIBRARIES}"
	)	
endif()			



