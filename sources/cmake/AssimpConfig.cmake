SET(_assimp_HEADER_SEARCH_DIRS
"/usr/include"
"/usr/local/include"
"C:/Program Files (x86)/assimp" )
# check environment variable
SET(_assimp_ENV_ROOT_DIR "$ENV{ASSIMP_ROOT_DIR}")
IF(NOT ASSIMP_ROOT_DIR AND _assimp_ENV_ROOT_DIR)
	SET(ASSIMP_ROOT_DIR "${_assimp_ENV_ROOT_DIR}")
ENDIF(NOT ASSIMP_ROOT_DIR AND _assimp_ENV_ROOT_DIR)
# put user specified location at beginning of search
IF(ASSIMP_ROOT_DIR)
	SET(_assimp_HEADER_SEARCH_DIRS "${ASSIMP_ROOT_DIR}"
	"${ASSIMP_ROOT_DIR}/include"
	${_assimp_HEADER_SEARCH_DIRS})
ENDIF(ASSIMP_ROOT_DIR)
# locate header
FIND_PATH(ASSIMP_INCLUDE_DIR "include/assimp/Importer.hpp"
PATHS ${_assimp_HEADER_SEARCH_DIRS})
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ASSIMP DEFAULT_MSG
ASSIMP_INCLUDE_DIR)
IF(ASSIMP_FOUND)
	SET(ASSIMP_INCLUDE_DIRS "${ASSIMP_INCLUDE_DIR}/include")
	SET(ASSIMP_LIB_DIR "${ASSIMP_INCLUDE_DIR}/lib64")
	MESSAGE(STATUS "ASSIMP_INCLUDE_DIRS = ${ASSIMP_INCLUDE_DIR}")
ELSE(ASSIMP_FOUND)
	MESSAGE(FATAL_ERROR "ASSIMP not found!")
ENDIF(ASSIMP_FOUND)