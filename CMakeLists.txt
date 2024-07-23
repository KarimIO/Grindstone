cmake_minimum_required(VERSION 3.5)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	if(DEFINED ENV{VCPKG_ROOT})
		set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	else()
		set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	endif()
endif()

include("${CMAKE_TOOLCHAIN_FILE}")

project(Grindstone VERSION 1.0.0 LANGUAGES CXX CSharp)
enable_language(CSharp)

set(CMAKE_CXX_STANDARD 17)

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(CODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources/code")
set(COMMON_DIR "${CODE_DIR}/Common")
set(ENGINECORE_DIR "${CODE_DIR}/EngineCore")
set(EDITORCONFIG "${CODE_DIR}/.editorconfig")
set(BUILD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CORE_UTILS ${ENGINECORE_DIR}/Logger.cpp ${ENGINECORE_DIR}/Logger.hpp)

set(ENGINE_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sources/code/EngineCore)
file(GLOB_RECURSE SOURCE_REFLECTION "${ENGINE_CORE_DIR}/Reflection/*.cpp")
file(GLOB_RECURSE HEADER_REFLECTION "${ENGINE_CORE_DIR}/Reflection/*.hpp")

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

option(USE_OPENGL "Include OpenGL?" ON)
if (USE_OPENGL)
add_subdirectory(${PLUGIN_DIR}/GraphicsOpenGL)
endif()

option(USE_VULKAN "Include Vulkan?" ON)
if (USE_VULKAN)
	add_subdirectory(${PLUGIN_DIR}/GraphicsVulkan)
endif()

option(USE_BULLET_PHYSICS "Include Bullet Physics?" ON)
if (USE_BULLET_PHYSICS)
	add_subdirectory(${PLUGIN_DIR}/PhysicsBullet)
endif()

option(USE_AUDIO_OPENAL "Include OpenAL Audio?" ON)
if (USE_AUDIO_OPENAL)
	add_subdirectory(${PLUGIN_DIR}/AudioOpenAL)
endif()

option(USE_ANIMATION "Include Animation?" OFF)
if (USE_ANIMATION)
	add_subdirectory(${PLUGIN_DIR}/Animation)
endif()

option(USE_RENDERABLES_3D "Include Renderables 3D?" ON)
if (USE_RENDERABLES_3D)
	add_subdirectory(${PLUGIN_DIR}/Renderables3D)
endif()

option(USE_EDITOR_AUDIO_IMPORTER "Include EditorAudioImporter?" ON)
if (USE_EDITOR_AUDIO_IMPORTER)
	add_subdirectory(${PLUGIN_DIR}/EditorAudioImporter)
endif()

option(USE_EDITOR_MATERIAL_IMPORTER "Include EditorMaterialImporter?" ON)
if (USE_EDITOR_MATERIAL_IMPORTER)
	add_subdirectory(${PLUGIN_DIR}/EditorMaterialImporter)
endif()

option(USE_EDITOR_MODEL_IMPORTER "Include EditorModelImporter?" ON)
if (USE_EDITOR_MODEL_IMPORTER)
	add_subdirectory(${PLUGIN_DIR}/EditorModelImporter)
endif()

option(USE_EDITOR_SHADER_IMPORTER "Include EditorShaderImporter?" ON)
if (USE_EDITOR_SHADER_IMPORTER)
	add_subdirectory(${PLUGIN_DIR}/EditorShaderImporter)
endif()

option(USE_EDITOR_TEXTURE_IMPORTER "Include EditorTextureImporter?" ON)
if (USE_EDITOR_TEXTURE_IMPORTER)
	add_subdirectory(${PLUGIN_DIR}/EditorTextureImporter)
endif()

option(USE_CSHARP "Include CSharp?" ON)
if (USE_CSHARP)
	add_subdirectory(${PLUGIN_DIR}/ScriptCSharp)
	include_external_msproject(
		GrindstoneCSharpCore GrindstoneCore.csproj
		TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
	)
endif()
