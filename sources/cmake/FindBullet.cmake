# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindBullet
# ----------
#
# Try to find the Bullet physics engine
#
#
#
# ::
#
#   This module defines the following variables
#
#
#
# ::
#
#   BULLET_FOUND - Was bullet found
#   BULLET_INCLUDE_DIRS - the Bullet include directories
#   BULLET_LIBRARIES - Link to this, by default it includes
#                      all bullet components (Dynamics,
#                      Collision, LinearMath, & SoftBody)
#
#
#
# ::
#
#   This module accepts the following variables
#
#
#
# ::
#
#   BULLET_ROOT - Can be set to bullet install path or Windows build path

set(BULLET_ROOT CACHE PATH "BULLET root directory")

if(WIN32)
	# Find path of each library
	find_path(BULLET_INCLUDE_DIR
		NAMES
			btBulletCollisionCommon.h
		HINTS
			${BULLET_ROOT}/src
	)

	find_path(BULLET_LIBRARY_DIR
		NAMES
			BulletCollision.lib BulletDynamics.lib BulletSoftBody.lib  LinearMath.lib
		HINTS
			${BULLET_ROOT}/bin
	)

	set(bullet_LIBRARIES ${BULLET_LIBRARY_DIR}/BulletCollision.lib ${BULLET_LIBRARY_DIR}/BulletDynamics.lib ${BULLET_LIBRARY_DIR}/LinearMath.lib)
	set(bullet_LIBRARIES_DEBUG ${BULLET_LIBRARY_DIR}/BulletCollision_d.lib ${BULLET_LIBRARY_DIR}/BulletDynamics_d.lib ${BULLET_LIBRARY_DIR}/LinearMath_d.lib)

	if (BULLET_INCLUDE_DIR AND bullet_LIBRARIES)
		SET(bullet_FOUND TRUE)
	ENDIF (BULLET_INCLUDE_DIR AND bullet_LIBRARIES)

	if (bullet_FOUND)
		if (NOT bullet_FIND_QUIETLY)
		message(STATUS "Found bullet library.")
		endif (NOT bullet_FIND_QUIETLY)
	else (bullet_FOUND)
		if (NOT BULLET_INCLUDE_DIR)
			message(STATUS "Could not find Bullet Includes")
		endif(NOT BULLET_INCLUDE_DIR)
		if (NOT bullet_LIBRARIES)
			message(STATUS "Could not find Bullet Libraries")
		endif(NOT bullet_LIBRARIES)

		if (bullet_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find bullet library")
		endif (bullet_FIND_REQUIRED)
	endif (bullet_FOUND)
else(WIN32)
  macro(_FIND_BULLET_LIBRARY _var)
	find_library(${_var}
	  NAMES
		  ${ARGN}
	  HINTS
		  ${BULLET_ROOT}
		  ${BULLET_ROOT}/lib/Release
		  ${BULLET_ROOT}/lib/Debug
		  ${BULLET_ROOT}/out/release8/libs
		  ${BULLET_ROOT}/out/debug8/libs
	  PATH_SUFFIXES lib
	)
	mark_as_advanced(${_var})
  endmacro()

  macro(_BULLET_APPEND_LIBRARIES _list _release)
	set(_debug ${_release}_DEBUG)
	if(${_debug})
		set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
	else()
		set(${_list} ${${_list}} ${${_release}})
	endif()
  endmacro()

  find_path(BULLET_INCLUDE_DIR NAMES bullet/btBulletCollisionCommon.h
	HINTS
	/usr/local/include/ /usr/include/
  )
  set(BULLET_INCLUDE_DIR ${BULLET_INCLUDE_DIR}/bullet)
  message(STATUS "Found Bullet Includes Directory: " ${BULLET_INCLUDE_DIR})

  # Find the libraries

  _FIND_BULLET_LIBRARY(BULLET_DYNAMICS_LIBRARY        BulletDynamics)
  _FIND_BULLET_LIBRARY(BULLET_DYNAMICS_LIBRARY_DEBUG  BulletDynamics_Debug BulletDynamics_d)
  _FIND_BULLET_LIBRARY(BULLET_COLLISION_LIBRARY       BulletCollision)
  _FIND_BULLET_LIBRARY(BULLET_COLLISION_LIBRARY_DEBUG BulletCollision_Debug BulletCollision_d)
  _FIND_BULLET_LIBRARY(BULLET_MATH_LIBRARY            BulletMath LinearMath)
  _FIND_BULLET_LIBRARY(BULLET_MATH_LIBRARY_DEBUG      BulletMath_Debug BulletMath_d LinearMath_Debug LinearMath_d)
  _FIND_BULLET_LIBRARY(BULLET_SOFTBODY_LIBRARY        BulletSoftBody)
  _FIND_BULLET_LIBRARY(BULLET_SOFTBODY_LIBRARY_DEBUG  BulletSoftBody_Debug BulletSoftBody_d)


  include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Bullet DEFAULT_MSG
	  BULLET_DYNAMICS_LIBRARY BULLET_COLLISION_LIBRARY BULLET_MATH_LIBRARY
	  BULLET_SOFTBODY_LIBRARY BULLET_INCLUDE_DIR)

  set(BULLET_INCLUDE_DIRS ${BULLET_INCLUDE_DIR})
  if(BULLET_FOUND)
	_BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_DYNAMICS_LIBRARY)
	_BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_COLLISION_LIBRARY)
	_BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_MATH_LIBRARY)
	_BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_SOFTBODY_LIBRARY)
  endif()
endif(WIN32)