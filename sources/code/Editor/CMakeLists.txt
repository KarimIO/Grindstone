set(EDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCE_MAIN
	Main.cpp
	../Common/Utilities/ModuleLoading.cpp ../EngineCore/Utils/Utilities.cpp
	../Common/MetaFile.cpp ../Common/MetaFile.hpp ../Common/Uuid.hpp
	../EngineCore/ECS/Entity.cpp
	FileManager.cpp FileManager.hpp FileDirectory.hpp
	EditorManager.cpp EditorManager.hpp
	Selection.cpp Selection.hpp
	EditorCamera.cpp EditorCamera.hpp
	Camera.cpp Camera.hpp
)

file(GLOB_RECURSE SOURCE_EDITOR_COMMANDS "${EDITOR_DIR}/Commands/*.cpp")
file(GLOB_RECURSE HEADER_EDITOR_COMMANDS "${EDITOR_DIR}/Commands/*.hpp")
source_group("Commands\\Source Files" FILES ${SOURCE_EDITOR_COMMANDS})
source_group("Commands\\Header Files" FILES ${HEADER_EDITOR_COMMANDS})

file(GLOB_RECURSE SOURCE_IMPORTERS "${EDITOR_DIR}/Importers/*.cpp")
file(GLOB_RECURSE HEADER_IMPORTERS "${EDITOR_DIR}/Importers/*.hpp")
source_group("Importers\\Source Files" FILES ${SOURCE_IMPORTERS})
source_group("Importers\\Header Files" FILES ${HEADER_IMPORTERS})

file(GLOB_RECURSE SOURCE_IMGUI_EDITOR "${EDITOR_DIR}/ImguiEditor/*.cpp")
file(GLOB_RECURSE HEADER_IMGUI_EDITOR "${EDITOR_DIR}/ImguiEditor/*.hpp")
source_group("Imgui Editor\\Source Files" FILES ${SOURCE_IMGUI_EDITOR})
source_group("Imgui Editor\\Header Files" FILES ${HEADER_IMGUI_EDITOR})

source_group("Imgui Editor\\Imgui\\Source Files" FILES ${SOURCES_IMGUI})

set(EDITOR_SOURCES
	${SOURCE_MAIN}
	${SOURCE_IMGUI_EDITOR}
	${SOURCE_IMPORTERS}
	${SOURCE_EDITOR_COMMANDS}
)

set(EDITOR_HEADERS
	${HEADER_IMGUI_EDITOR}
	${HEADER_IMPORTERS}
	${HEADER_EDITOR_COMMANDS}
)

find_package(Assimp CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_c CONFIG REQUIRED)

add_executable(Editor WIN32 ${EDITOR_HEADERS} ${EDITOR_SOURCES})

set_property(TARGET Editor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BUILD_DIRECTORY})
target_compile_definitions(Editor PUBLIC NOMINMAX)
target_include_directories(Editor PUBLIC ${CODE_DIR})

find_library(SHADERC_LIBRARY
	NAMES
		shaderc
	HINTS
		${CMAKE_FIND_ROOT_PATH}
	PATHS
		${CMAKE_FIND_ROOT_PATH}
	PATH_SUFFIXES
		"debug/lib"
		"lib"
		"local/lib"
)

find_library(SHADERC_UTIL_LIBRARY
	NAMES
		shaderc_util
	HINTS
		${CMAKE_FIND_ROOT_PATH}
	PATHS
		${CMAKE_FIND_ROOT_PATH}
	PATH_SUFFIXES
		"debug/lib"
		"lib"
		"local/lib"
)

find_library(SPIRV_Tools
	NAMES
		SPIRV-Tools
	HINTS
		${CMAKE_FIND_ROOT_PATH}
	PATHS
		${CMAKE_FIND_ROOT_PATH}
	PATH_SUFFIXES
		"debug/lib"
		"lib"
		"local/lib"
)

find_library(SPIRV_Tools_Opt
	NAMES
		SPIRV-Tools-opt
	HINTS
		${CMAKE_FIND_ROOT_PATH}
	PATHS
		${CMAKE_FIND_ROOT_PATH}
	PATH_SUFFIXES
		"debug/lib"
		"lib"
		"local/lib"
)

target_link_libraries(
	Editor
	Rpcrt4.lib
	efsw::efsw
	spirv-cross-c spirv-cross-core spirv-cross-cpp spirv-cross-glsl spirv-cross-reflect
	HLSL SPIRV glslang OGLCompiler
	${SHADERC_LIBRARY} ${SHADERC_UTIL_LIBRARY} ${SPIRV_Tools} ${SPIRV_Tools_Opt}
	spirv-cross-reflect
	assimp::assimp
	unofficial::gl3w::gl3w glm::glm opengl32
	imgui::imgui
	${CMAKE_DL_LIBS}
)
