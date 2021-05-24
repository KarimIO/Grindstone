set(EDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCE_MAIN
	Main.cpp
	../Common/Utilities/ModuleLoading.cpp
	EditorManager.cpp EditorManager.hpp
	EditorCamera.cpp EditorCamera.hpp
	Camera.cpp Camera.hpp
)

file(GLOB_RECURSE SOURCE_CONVERTERS "${EDITOR_DIR}/Converters/*.cpp")
file(GLOB_RECURSE HEADER_CONVERTERS "${EDITOR_DIR}/Converters/*.hpp")
source_group("Converters\\Source Files" FILES ${SOURCE_CONVERTERS})
source_group("Converters\\Header Files" FILES ${HEADER_CONVERTERS})

file(GLOB_RECURSE SOURCE_IMGUI_EDITOR "${EDITOR_DIR}/ImguiEditor/*.cpp")
file(GLOB_RECURSE HEADER_IMGUI_EDITOR "${EDITOR_DIR}/ImguiEditor/*.hpp")
source_group("Imgui Editor\\Source Files" FILES ${SOURCE_IMGUI_EDITOR})
source_group("Imgui Editor\\Header Files" FILES ${HEADER_IMGUI_EDITOR})

set(SOURCE_IMGUI
	${DEPS_DIR}/GL/gl3w.cpp
	${DEPS_DIR}/GL/gl3w.h
	${DEPS_DIR}/GL/glcorearb.h
	${DEPS_DIR}/imgui/imgui.cpp
	${DEPS_DIR}/imgui/imgui_demo.cpp
	${DEPS_DIR}/imgui/imgui_draw.cpp
	${DEPS_DIR}/imgui/imgui_tables.cpp
	${DEPS_DIR}/imgui/imgui_widgets.cpp
	${DEPS_DIR}/imgui/backends/imgui_impl_win32.cpp
	${DEPS_DIR}/imgui/backends/imgui_impl_opengl3.cpp
	${DEPS_DIR}/imgui/misc/cpp/imgui_stdlib.cpp
)
source_group("Imgui Editor\\Imgui\\Source Files" FILES ${SOURCES_IMGUI})

find_package(assimp REQUIRED)

set(EDITOR_SOURCES
	${SOURCE_MAIN}
	${SOURCE_IMGUI}
	${SOURCE_IMGUI_EDITOR}
	${SOURCE_CONVERTERS}
)

set(EDITOR_HEADERS
	${HEADER_IMGUI_EDITOR}
	${HEADER_CONVERTERS}
)

add_executable(Editor ${EDITOR_HEADERS} ${EDITOR_SOURCES})

set_property(TARGET Editor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BUILD_DIRECTORY})

target_include_directories(Editor
    PUBLIC
	../
	${ASSIMP_INCLUDE_DIR}
	${DEPS_DIR}
	${DEPS_DIR}/imgui
	${DEPS_DIR}/entt/src
)

target_link_libraries(Editor ${assimp_LIBRARIES} ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS})