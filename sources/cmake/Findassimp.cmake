if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ASSIMP_ARCHITECTURE "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(ASSIMP_ARCHITECTURE "86")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

if(WIN32)
	set(ASSIMP_ROOT_DIR CACHE PATH "ASSIMP root directory")

	# Find path of each library
	find_path(ASSIMP_INCLUDE_DIR
		NAMES
			assimp/anim.h
		HINTS
			${ASSIMP_ROOT_DIR}/include ${ASSIMP_ROOT_DIR}/include/assimp
	)

	find_path(ASSIMP_LIBRARY_DIR
		NAMES
			assimp.lib
		HINTS
			${ASSIMP_ROOT_DIR}/lib/x${ASSIMP_ARCHITECTURE} ${ASSIMP_ROOT_DIR}/lib${ASSIMP_ARCHITECTURE}
	)

	set(assimp_LIBRARIES assimp)

	if (assimp_LIBRARIES AND assimp_LIBRARIES)
		SET(assimp_FOUND TRUE)
	ENDIF (assimp_LIBRARIES AND assimp_LIBRARIES)

	if (assimp_FOUND)
		if (NOT assimp_FIND_QUIETLY)
		message(STATUS "Found asset importer library: ${assimp_LIBRARIES}")
		endif (NOT assimp_FIND_QUIETLY)
	else (assimp_FOUND)
		if (NOT assimp_LIBRARIES)
			message(STATUS "Could not find AssImp Includes")
		endif(NOT assimp_LIBRARIES)
		if (NOT assimp_LIBRARIES)
			message(STATUS "Could not find AssImp Libraries")
		endif(NOT assimp_LIBRARIES)

		if (assimp_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find asset importer library")
		endif (assimp_FIND_REQUIRED)
	endif (assimp_FOUND)

else(WIN32)

	find_path(
	  assimp_INCLUDE_DIRS
	  NAMES assimp/postprocess.h assimp/scene.h assimp/version.h assimp/config.h assimp/cimport.h
	  PATHS /usr/local/include/ /usr/include/
	)

	find_library(
	  assimp_LIBRARIES
	  NAMES assimp
	  PATHS /usr/local/lib/ /usr/lib/ /usr/lib/x86_64-linux-gnu
	)

	if (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)
	  SET(assimp_FOUND TRUE)
	ENDIF (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)

	if (assimp_FOUND)
	  if (NOT assimp_FIND_QUIETLY)
		message(STATUS "Found asset importer library: ${assimp_LIBRARIES}")
	  endif (NOT assimp_FIND_QUIETLY)
	else (assimp_FOUND)
		if (NOT assimp_INCLUDE_DIRS)
			message(STATUS "Could not find AssImp Includes")
		endif(NOT assimp_INCLUDE_DIRS)
		if (NOT assimp_LIBRARIES)
			message(STATUS "Could not find AssImp Libraries")
		endif(NOT assimp_LIBRARIES)

	  if (assimp_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find asset importer library")
	  endif (assimp_FIND_REQUIRED)
	endif (assimp_FOUND)
	
endif(WIN32)