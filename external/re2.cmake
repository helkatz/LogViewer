if(TARGET re2)
	return()
endif()
set(RE2_DIR /Data/binaries/external/re2)

add_library(re2 STATIC IMPORTED GLOBAL)

set_target_properties(re2 PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES ${RE2_DIR}/include	
	IMPORTED_LOCATION ${RE2_DIR}/lib/re2.lib
	IMPORTED_LOCATION_DEBUG ${RE2_DIR}/lib/re2d.lib
	)
