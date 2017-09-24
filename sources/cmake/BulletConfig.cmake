SET(_bullet_HEADER_SEARCH_DIRS
"/usr/include"
"/usr/local/include"
"C:/Program Files (x86)/bullet" )
# check environment variable
SET(_bullet_ENV_ROOT_DIR "$ENV{BULLET_ROOT_DIR}")
IF(NOT BULLET_ROOT_DIR AND _bullet_ENV_ROOT_DIR)
	SET(BULLET_ROOT_DIR "${_bullet_ENV_ROOT_DIR}")
ENDIF(NOT BULLET_ROOT_DIR AND _bullet_ENV_ROOT_DIR)
# put user specified location at beginning of search
IF(BULLET_ROOT_DIR)
	SET(_bullet_HEADER_SEARCH_DIRS "${BULLET_ROOT_DIR}"
	"${BULLET_ROOT_DIR}/include"
	${_bullet_HEADER_SEARCH_DIRS})
ENDIF(BULLET_ROOT_DIR)
# locate header
FIND_PATH(BULLET_INCLUDE_DIR "bullet/btBulletCollisionCommon.h"
PATHS ${_bullet_HEADER_SEARCH_DIRS})
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BULLET DEFAULT_MSG
BULLET_INCLUDE_DIR)
IF(BULLET_FOUND)
	SET(BULLET_INCLUDE_DIRS "${BULLET_INCLUDE_DIR}/src")
	set(BULLET_LIB_DIR "${BULLET_INCLUDE_DIR}/bin")
	if (WIN32)
		set(BULLET_DEBUG_LIBS ${BULLET_LIB_DIR}/BulletCollision_vs2010_x64_debug.lib ${BULLET_LIB_DIR}/BulletDynamics_vs2010_x64_debug.lib ${BULLET_LIB_DIR}/LinearMath_vs2010_x64_debug.lib)

		set(BULLET_RELEASE_LIBS ${BULLET_LIB_DIR}/BulletCollision_vs2010_x64_release.lib ${BULLET_LIB_DIR}/BulletDynamics_vs2010_x64_release.lib ${BULLET_LIB_DIR}/LinearMath_vs2010_x64_release.lib)
	else ()
		set(BULLET_LIBS ${BULLET_LIB_DIR}/libBulletDynamics.a ${BULLET_LIB_DIR}/libBulletCollision.a ${BULLET_LIB_DIR}/libLinearMath.a)
	endif()

	MESSAGE(STATUS "BULLET_INCLUDE_DIR = ${BULLET_INCLUDE_DIR}")
ELSE(BULLET_FOUND)
	MESSAGE(FATAL_ERROR "BULLET not found!")
ENDIF(BULLET_FOUND)