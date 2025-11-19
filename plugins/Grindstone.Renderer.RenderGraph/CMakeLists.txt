set(RENDER_GRAPH_BASE ${PLUGIN_DIR}/Grindstone.Renderer.RenderGraph)
set(RENDER_GRAPH_INCLUDES ${RENDER_GRAPH_BASE}/include)
set(RENDER_GRAPH_BIN ${RENDER_GRAPH_BASE}/bin)
set(RENDER_GRAPH_LIB ${RENDER_GRAPH_BASE}/lib)
set(SRC ${RENDER_GRAPH_BASE}/source)
set(INC ${RENDER_GRAPH_BASE}/include)

set(RENDER_GRAPH_SOURCES ${SRC}/RenderGraphPass.cpp ${SRC}/RenderGraphBuilder.cpp ${SRC}/RenderGraph.cpp ${SRC}/EntryPoint.cpp)
set(RENDER_GRAPH_HEADERS ${INC}/RenderGraphPass.hpp ${INC}/RenderGraphBuilder.hpp ${INC}/RenderGraph.hpp ${INC}/GpuQueue.hpp ${INC}/AttachmentInfo.hpp ${INC}/BufferInfo.hpp)

add_library(
	PluginRenderGraph
	MODULE
	${RENDER_GRAPH_SOURCES}
	${RENDER_GRAPH_HEADERS}
	${CORE_UTILS}
)

set_property(TARGET PluginRenderGraph PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_definitions(PluginRenderGraph PUBLIC RENDER_GRAPH_DLL GLM_ENABLE_EXPERIMENTAL)
set_target_properties(PluginRenderGraph PROPERTIES FOLDER Plugins/Renderer)

find_path(GLM_INCLUDE_DIRS "glm/glm.hpp")

set_target_properties(PluginRenderGraph PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${RENDER_GRAPH_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${RENDER_GRAPH_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${RENDER_GRAPH_LIB}"
)
target_link_libraries(
	PluginRenderGraph
	${CORE_LIBS}
	Common
)

target_include_directories(
	PluginRenderGraph PUBLIC
	${CODE_DIR}
	${GLM_INCLUDE_DIRS}
	${PLUGIN_DIR}
)

target_precompile_headers(
	PluginRenderGraph PUBLIC
	${INC}/pch.hpp
)

target_compile_definitions(PluginRenderGraph PUBLIC NOMINMAX)

message(STATUS
"PluginRenderGraph provides CMake targets:

- RENDER_GRAPH_BASE => ${RENDER_GRAPH_BASE}
- RENDER_GRAPH_INCLUDES => ${RENDER_GRAPH_INCLUDES}
- RENDER_GRAPH_BIN => ${RENDER_GRAPH_BIN}
- RENDER_GRAPH_LIB => ${RENDER_GRAPH_LIB}

")
