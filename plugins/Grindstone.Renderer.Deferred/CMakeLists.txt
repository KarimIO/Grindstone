set(RENDERER_DEFERRED_BASE ${PLUGIN_DIR}/Grindstone.Renderer.Deferred)
set(RENDERER_DEFERRED_INCLUDES ${RENDERER_DEFERRED_BASE}/include)
set(RENDERER_DEFERRED_BIN ${RENDERER_DEFERRED_BASE}/bin)
set(RENDERER_DEFERRED_LIB ${RENDERER_DEFERRED_BASE}/lib)
set(SRC ${RENDERER_DEFERRED_BASE}/source)
set(INC ${RENDERER_DEFERRED_BASE}/include)

set(RENDERER_DEFERRED_SOURCES ${SRC}/DeferredRendererFactory.cpp ${SRC}/DeferredRenderer.cpp ${SRC}/EntryPoint.cpp)
set(RENDERER_DEFERRED_HEADERS ${INC}/DeferredRendererFactory.hpp ${INC}/DeferredRenderer.hpp ${INC}/DeferredRendererCommon.hpp)

file(GLOB_RECURSE SOURCE_PASSES "${SRC}/Passes/*.cpp")
file(GLOB_RECURSE HEADER_PASSES "${INC}/Passes/*.hpp")
source_group("Source Files\\Passes" FILES ${SOURCE_PASSES})
source_group("Header Files\\Passes" FILES ${HEADER_PASSES})

add_library(
	PluginRendererDeferred
	MODULE
	${RENDERER_DEFERRED_SOURCES}
	${RENDERER_DEFERRED_HEADERS}
	${SOURCE_PASSES}
	${HEADER_PASSES}
	${CORE_UTILS}
)

set_property(TARGET PluginRendererDeferred PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_definitions(PluginRendererDeferred PUBLIC RENDERER_DEFERRED_DLL GLM_ENABLE_EXPERIMENTAL)
set_target_properties(PluginRendererDeferred PROPERTIES FOLDER Plugins/Renderer)

find_path(GLM_INCLUDE_DIRS "glm/glm.hpp")

set_target_properties(PluginRendererDeferred PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${RENDERER_DEFERRED_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${RENDERER_DEFERRED_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${RENDERER_DEFERRED_LIB}"
)
target_link_libraries(
	PluginRendererDeferred
	${CORE_LIBS}
	Common
)

target_include_directories(
	PluginRendererDeferred PUBLIC
	${CODE_DIR}
	${GLM_INCLUDE_DIRS}
	${PLUGIN_DIR}
)

target_precompile_headers(
	PluginRendererDeferred PUBLIC
	${INC}/pch.hpp
)

target_compile_definitions(PluginRendererDeferred PUBLIC NOMINMAX)

message(STATUS
"PluginRendererDeferred provides CMake targets:

- RENDERER_DEFERRED_BASE => ${RENDERER_DEFERRED_BASE}
- RENDERER_DEFERRED_INCLUDES => ${RENDERER_DEFERRED_INCLUDES}
- RENDERER_DEFERRED_BIN => ${RENDERER_DEFERRED_BIN}
- RENDERER_DEFERRED_LIB => ${RENDERER_DEFERRED_LIB}

")
