set(EDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCE_MAIN
	Main.cpp
	../Common/Utilities/ModuleLoading.cpp ../EngineCore/Utils/Utilities.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp ${COMMON_DIR}/ResourcePipeline/MetaFile.hpp ${COMMON_DIR}/ResourcePipeline/Uuid.hpp  ${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	../EngineCore/ECS/Entity.cpp
	FileManager.cpp FileManager.hpp FileDirectory.hpp
	EditorManager.cpp EditorManager.hpp
	Selection.cpp Selection.hpp
	EditorCamera.cpp EditorCamera.hpp
	Camera.cpp Camera.hpp
)

file(GLOB_RECURSE SOURCE_SCRIPT_BUILDER_EDITOR "${EDITOR_DIR}/ScriptBuilder/*.cpp")
file(GLOB_RECURSE HEADER_SCRIPT_BUILDER_EDITOR "${EDITOR_DIR}/ScriptBuilder/*.hpp")
source_group("Script Builder\\Source Files" FILES ${SOURCE_SCRIPT_BUILDER_EDITOR})
source_group("Script Builder\\Header Files" FILES ${HEADER_SCRIPT_BUILDER_EDITOR})

file(GLOB_RECURSE SOURCE_EDITOR_COMMANDS "${EDITOR_DIR}/Commands/*.cpp")
file(GLOB_RECURSE HEADER_EDITOR_COMMANDS "${EDITOR_DIR}/Commands/*.hpp")
source_group("Commands\\Source Files" FILES ${SOURCE_EDITOR_COMMANDS})
source_group("Commands\\Header Files" FILES ${HEADER_EDITOR_COMMANDS})

file(GLOB_RECURSE SOURCE_IMPORTERS "${EDITOR_DIR}/Importers/*.cpp")
file(GLOB_RECURSE HEADER_IMPORTERS "${EDITOR_DIR}/Importers/*.hpp")
source_group("Importers\\Source Files" FILES ${SOURCE_IMPORTERS})
source_group("Importers\\Header Files" FILES ${HEADER_IMPORTERS})

file(GLOB_RECURSE SOURCE_IMGUI_PREFS_EDITOR "${EDITOR_DIR}/ImguiEditor/Settings/*.cpp")
file(GLOB_RECURSE HEADER_IMGUI_PREFS_EDITOR "${EDITOR_DIR}/ImguiEditor/Settings/*.hpp")
source_group("Imgui Editor\\Settings\\Source Files" FILES ${SOURCE_IMGUI_PREFS_EDITOR})
source_group("Imgui Editor\\Settings\\Header Files" FILES ${HEADER_IMGUI_PREFS_EDITOR})

file(GLOB_RECURSE SOURCE_IMGUI_PREFS_USER_EDITOR "${EDITOR_DIR}/ImguiEditor/UserSettings/*.cpp")
file(GLOB_RECURSE HEADER_IMGUI_PREFS_USER_EDITOR "${EDITOR_DIR}/ImguiEditor/UserSettings/*.hpp")
source_group("Imgui Editor\\Settings\\User\\Source Files" FILES ${SOURCE_IMGUI_PREFS_USER_EDITOR})
source_group("Imgui Editor\\Settings\\User\\Header Files" FILES ${HEADER_IMGUI_PREFS_USER_EDITOR})

file(GLOB_RECURSE SOURCE_IMGUI_PREFS_PROJECT_EDITOR "${EDITOR_DIR}/ImguiEditor/ProjectSettings/*.cpp")
file(GLOB_RECURSE HEADER_IMGUI_PREFS_PROJECT_EDITOR "${EDITOR_DIR}/ImguiEditor/ProjectSettings/*.hpp")
source_group("Imgui Editor\\Settings\\Project\\Source Files" FILES ${SOURCE_IMGUI_PREFS_PROJECT_EDITOR})
source_group("Imgui Editor\\Settings\\Project\\Header Files" FILES ${HEADER_IMGUI_PREFS_PROJECT_EDITOR})

file(GLOB_RECURSE SOURCE_IMGUI_COMPONENtS_EDITOR "${EDITOR_DIR}/ImguiEditor/Components/*.cpp")
file(GLOB_RECURSE HEADER_IMGUI_COMPONENtS_EDITOR "${EDITOR_DIR}/ImguiEditor/Components/*.hpp")
source_group("Imgui Editor\\Components\\Source Files" FILES ${SOURCE_IMGUI_COMPONENtS_EDITOR})
source_group("Imgui Editor\\Components\\Header Files" FILES ${HEADER_IMGUI_COMPONENtS_EDITOR})

file(GLOB_RECURSE SOURCE_IMGUI_EDITOR "${EDITOR_DIR}/ImguiEditor/*.cpp")
file(GLOB_RECURSE HEADER_IMGUI_EDITOR "${EDITOR_DIR}/ImguiEditor/*.hpp")
source_group("Imgui Editor\\Source Files" FILES ${SOURCE_IMGUI_EDITOR})
source_group("Imgui Editor\\Header Files" FILES ${HEADER_IMGUI_EDITOR})

source_group("Imgui Editor\\Imgui\\Source Files" FILES ${SOURCES_IMGUI})

set(EDITOR_SOURCES
	${SOURCE_MAIN}
	${SOURCE_IMPORTERS}
	${SOURCE_EDITOR_COMMANDS}
	${SOURCE_IMGUI_PREFS_EDITOR}
	${SOURCE_IMGUI_PREFS_USER_EDITOR}
	${SOURCE_IMGUI_PREFS_PROJECT_EDITOR}
	${SOURCE_IMGUI_COMPONENTS_EDITOR}
	${SOURCE_SCRIPT_BUILDER_EDITOR}
	${SOURCE_IMGUI_EDITOR}
)

set(EDITOR_HEADERS
	${HEADER_IMPORTERS}
	${HEADER_EDITOR_COMMANDS}
	${HEADER_IMGUI_PREFS_EDITOR}
	${HEADER_IMGUI_PREFS_USER_EDITOR}
	${HEADER_IMGUI_PREFS_PROJECT_EDITOR}
	${HEADER_IMGUI_COMPONENTS_EDITOR}
	${HEADER_SCRIPT_BUILDER_EDITOR}
	${HEADER_IMGUI_EDITOR}
)

find_package(Assimp CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_include.h")
find_path(FMT_INCLUDE_DIRS "fmt/format.h")

find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_c CONFIG REQUIRED)
find_package(SPIRV-Tools-opt CONFIG REQUIRED)
find_package(SPIRV-Tools CONFIG REQUIRED)
add_library(SPIRV-Tools ALIAS SPIRV-Tools-static)
find_package(unofficial-shaderc_util CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(Editor ${EDITOR_HEADERS} ${EDITOR_SOURCES})

set_property(TARGET Editor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BUILD_DIRECTORY})

target_compile_definitions(Editor PUBLIC NOMINMAX)
target_include_directories(Editor PUBLIC ${CODE_DIR} ${STB_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS})

target_link_libraries(
	Editor
	Rpcrt4.lib
	efsw::efsw
	fmt::fmt
	spirv-cross-c spirv-cross-core spirv-cross-cpp spirv-cross-glsl spirv-cross-reflect
	glslang::SPIRV glslang::HLSL glslang::glslang glslang::OGLCompiler
	unofficial::shaderc::shaderc unofficial::shaderc_util::shaderc_util
	SPIRV-Tools-opt SPIRV-Tools-static
	spirv-cross-reflect
	assimp::assimp
	unofficial::gl3w::gl3w glm::glm opengl32
	imgui::imgui
	${CMAKE_DL_LIBS}
)
