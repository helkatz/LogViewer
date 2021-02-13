if(TARGET boost)
	return()
endif()
message("boost")
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(Boost_DEBUG              	 ON)
set(Boost_REALPATH           	ON)
#find_package(Boost 1.62.0  REQUIRED)
set(Boost_ROOT "/Data/binaries/external/boost/1_62_0")
set(Boost_INCLUDE_DIRS ${Boost_ROOT})
set(Boost_LIBRARYDIR ${Boost_ROOT}/lib32-msvc-14.0)
#set(Boost_LIBRARIES "${Boost_LIBRARYDIR}/boost_chrono-vc140-mt-1_62.lib")
#find_library(Boost_LIBRARIES_FOUND boost_chrono-vc140-mt-1_62 PATHS ${Boost_LIBRARYDIR})
message("
	Boost_LIBRARIES_FOUND = ${Boost_LIBRARIES_FOUND}
	Boost_LIBRARIES = ${Boost_LIBRARIES}
	Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}")
if(0)
	message("##### Build static")
	add_library(boost STATIC IMPORTED GLOBAL)

	set_target_properties(boost PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS}
		INTERFACE_LINK_LIBRARIES ${Boost_LIBRARIES})
else()
	#include_directories(${Boost_INCLUDE_DIRS})
	
	add_library(boost INTERFACE IMPORTED GLOBAL)
	#add_definitions("-DBOOST_ALL_NO_LIB")
	LINK_DIRECTORIES(${Boost_LIBRARYDIR})
	#target_link_libraries(boost INTERFACE_LINK_LIBRARIES ${Boost_LIBRARIES})
	set_target_properties(boost PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS}
		#LINK_INTERFACE_LIBRARIES ${Boost_LIBRARIES}
		#IMPORTED_LOCATION ${Boost_LIBRARIES}		
		#IMPORTED_LOCATION ${Boost_LIBRARIES}
	)
endif()

return()

