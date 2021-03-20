set(EDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCE_MAIN Main.cpp ../Common/Utilities/ModuleLoading.cpp)

set(HEADERS_CONVERTER EditorManager.hpp)
set(SOURCES_CONVERTER EditorManager.cpp)

file(GLOB_RECURSE SOURCE_IMGUI_EDITOR "${EDITOR_DIR}/ImguiEditor/*.cpp")
file(GLOB_RECURSE HEADER_IMGUI_EDITOR "${EDITOR_DIR}/ImguiEditor/*.hpp")
source_group("Imgui Editor\\Source Files" FILES ${SOURCE_IMGUI_EDITOR})
source_group("Imgui Editor\\Header Files" FILES ${HEADER_IMGUI_EDITOR})

set(SOURCES_IMGUI
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
)
source_group("Imgui Editor\\Imgui\\Source Files" FILES ${SOURCES_IMGUI})

find_package(assimp REQUIRED)

set(EDITOR_SOURCES
	${SOURCE_MAIN}
	${SOURCES_IMGUI}
	${SOURCE_IMGUI_EDITOR}
	${SOURCES_CONVERTER}
)

set(EDITOR_HEADERS
	${HEADERS_CONVERTER}
	${HEADER_IMGUI_EDITOR}
)

add_executable(Editor ${EDITOR_HEADERS} ${EDITOR_SOURCES})

set_property(TARGET Editor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BUILD_DIRECTORY})

target_include_directories(Editor
    PUBLIC
	../
	${ASSIMP_INCLUDE_DIR}
	${DEPS_DIR}
	${DEPS_DIR}/imgui
)

target_link_libraries(Editor ${assimp_LIBRARIES} ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS})