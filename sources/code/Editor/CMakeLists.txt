set(EDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCE_MAIN
	Main.cpp ${CODE_DIR}/NatvisFile.natvis
	${COMMON_DIR}/Containers/Containers.natvis
	${EDITOR_DIR}/EditorManagerInstance.cpp
	${ENGINE_CORE_DIR}/ECS/Entity.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	FileManager.cpp FileManager.hpp
	GitManager.cpp GitManager.hpp
	GizmoRenderer.cpp GizmoRenderer.hpp
	GridRenderer.cpp GridRenderer.hpp
	BuildProcess.cpp BuildProcess.hpp
	EditorManager.cpp EditorManager.hpp
	Selection.cpp Selection.hpp
	EditorCamera.cpp EditorCamera.hpp
	Camera.cpp Camera.hpp
	AssetRegistry.cpp AssetRegistry.hpp
	AssetPackSerializer.cpp AssetPackSerializer.hpp
	TaskSystem.cpp TaskSystem.hpp
	FileAssetLoader.cpp FileAssetLoader.hpp
	AssetTemplateRegistry.cpp AssetTemplateRegistry.hpp
	ThumbnailManager.cpp ThumbnailManager.hpp
	${CORE_UTILS}
)

file(GLOB_RECURSE SOURCE_PLUGIN_SYSTEM "${EDITOR_DIR}/PluginSystem/*.cpp")
file(GLOB_RECURSE HEADER_PLUGIN_SYSTEM "${EDITOR_DIR}/PluginSystem/*.hpp")
source_group("Plugin System\\Source Files" FILES ${SOURCE_PLUGIN_SYSTEM})
source_group("Plugin System\\Header Files" FILES ${HEADER_PLUGIN_SYSTEM})

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

file(GLOB_RECURSE SOURCE_IMGUI_COMPONENTS_EDITOR "${EDITOR_DIR}/ImguiEditor/Components/*.cpp")
file(GLOB_RECURSE HEADER_IMGUI_COMPONENTS_EDITOR "${EDITOR_DIR}/ImguiEditor/Components/*.hpp")
source_group("Imgui Editor\\Components\\Source Files" FILES ${SOURCE_IMGUI_COMPONENTS_EDITOR})
source_group("Imgui Editor\\Components\\Header Files" FILES ${HEADER_IMGUI_COMPONENTS_EDITOR})

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
	${SOURCE_PLUGIN_SYSTEM}
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
	${HEADER_PLUGIN_SYSTEM}
)

find_package(efsw CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

find_package(unofficial-imguizmo CONFIG REQUIRED)
find_package(unofficial-libgit2 CONFIG REQUIRED)

add_executable(Editor ${EDITOR_HEADERS} ${EDITOR_SOURCES})

set_target_properties(Editor PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${BUILD_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY "${BUILD_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY "${BUILD_DIRECTORY}"
)

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
	string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIGUPPER )
	set_target_properties(Editor PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${BUILD_DIRECTORY}/${OUTPUTCONFIG}"
		LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${BUILD_DIRECTORY}/${OUTPUTCONFIG}"
		ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${BUILD_DIRECTORY}/${OUTPUTCONFIG}"
	)
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set_property(TARGET Editor PROPERTY COMPILE_WARNING_AS_ERROR ON)
set_property(TARGET Editor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BUILD_DIRECTORY})

target_compile_definitions(Editor PUBLIC NOMINMAX GLM_ENABLE_EXPERIMENTAL EDITOR)
target_include_directories(Editor PUBLIC
	${CODE_DIR}
	${FMT_INCLUDE_DIRS}
	${PLUGIN_DIR}
)

target_precompile_headers(Editor PUBLIC pch.hpp)

target_link_libraries(
	Editor
	PUBLIC
	${CORE_LIBS}
	efsw::efsw
	unofficial::libgit2::libgit2
	unofficial::gl3w::gl3w
	glm::glm
	opengl32
	imgui::imgui unofficial::imguizmo::imguizmo
	${CMAKE_DL_LIBS}
	Common EditorCommon
)
