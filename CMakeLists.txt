cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	if(DEFINED ENV{VCPKG_ROOT})
		set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	else()
		set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	endif()
endif()

include("${CMAKE_TOOLCHAIN_FILE}")
set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

if (WIN32 AND CMAKE_GENERATOR MATCHES "Visual Studio")
	project(Grindstone VERSION 1.0.0 LANGUAGES CXX)
else()
	project(Grindstone VERSION 1.0.0 LANGUAGES CXX)
endif()

set(CMAKE_CXX_STANDARD 20)

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(CODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources/code")
set(COMMON_DIR "${CODE_DIR}/Common")
set(ENGINECORE_DIR "${CODE_DIR}/EngineCore")
set(EDITOR_COMMON_DIR "${CODE_DIR}/EditorCommon")
set(EDITOR_DIR "${CODE_DIR}/Editor")
set(EDITORCONFIG "${CODE_DIR}/.editorconfig")
set(BUILD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/plugins)

set(CORE_UTILS
	${ENGINECORE_DIR}/EngineCoreInstance.cpp
	${ENGINECORE_DIR}/Assets/AssetReference.cpp ${ENGINECORE_DIR}/Assets/AssetReference.hpp
	${ENGINECORE_DIR}/ECS/Entity.cpp ${ENGINECORE_DIR}/ECS/Entity.hpp
	${ENGINECORE_DIR}/Logger.cpp ${ENGINECORE_DIR}/Logger.hpp
	${ENGINECORE_DIR}/Utils/MemoryAllocator.cpp ${ENGINECORE_DIR}/Utils/MemoryAllocator.hpp
	${ENGINECORE_DIR}/Utils/DeferredDeletionQueue.cpp ${ENGINECORE_DIR}/Utils/DeferredDeletionQueue.hpp
)
set(CORE_LIBS fmt::fmt Rpcrt4.lib)

set(ENGINE_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sources/code/EngineCore)
file(GLOB_RECURSE SOURCE_REFLECTION "${ENGINE_CORE_DIR}/Reflection/*.cpp")
file(GLOB_RECURSE HEADER_REFLECTION "${ENGINE_CORE_DIR}/Reflection/*.hpp")

add_subdirectory(${COMMON_DIR})
add_subdirectory(${ENGINECORE_DIR})
add_subdirectory(${EDITOR_COMMON_DIR})
add_subdirectory(${EDITOR_DIR})
add_subdirectory(sources/code/ApplicationExecutable)

file(READ ${CMAKE_CURRENT_LIST_DIR}/Plugins.CmakeLists.txt PLUGIN_LIST)
string (REPLACE "\n" ";" PLUGIN_LIST "${PLUGIN_LIST}")
foreach (PluginEntry IN LISTS PLUGIN_LIST)
	if(NOT "${PluginEntry}" STREQUAL "")
		include(${PLUGIN_DIR}/${PluginEntry})
	endif()
endforeach()
