cmake_minimum_required(VERSION 3.5)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	if(DEFINED ENV{VCPKG_ROOT})
		set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	else()
		set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	endif()
endif()

include("${CMAKE_TOOLCHAIN_FILE}")

project(Grindstone VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(glm CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources/deps")
set(CODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources/code")
set(COMMON_DIR "${CODE_DIR}/Common")
set(BUILD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIRECTORY})

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
	string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BUILD_DIRECTORY} )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BUILD_DIRECTORY} )
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BUILD_DIRECTORY} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

add_subdirectory(sources/code/Editor)
add_subdirectory(sources/code/EngineCore)
add_subdirectory(sources/code/ApplicationExecutable)

set(PLUGIN_DIR sources/code/Plugins)

option(USE_OPENGL "Should the OpenGL Project be inclued?" ON)
option(USE_VULKAN "Should the Vulkan Project be inclued?" OFF)

if (USE_OPENGL)
	add_subdirectory(${PLUGIN_DIR}/GraphicsOpenGL)
endif()
if (USE_VULKAN)
	add_subdirectory(${PLUGIN_DIR}/GraphicsVulkan)
endif()
