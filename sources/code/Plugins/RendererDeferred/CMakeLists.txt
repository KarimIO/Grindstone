set(RENDERER_DEFERRED_SOURCES DeferredRendererFactory.cpp DeferredRenderer.cpp EntryPoint.cpp)
set(RENDERER_DEFERRED_HEADERS DeferredRendererFactory.hpp DeferredRenderer.hpp DeferredRendererCommon.hpp)

add_library(
	PluginRendererDeferred
	MODULE
	${RENDERER_DEFERRED_SOURCES}
	${RENDERER_DEFERRED_HEADERS}
	${CORE_UTILS}
)

set_property(TARGET PluginRendererDeferred PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_definitions(PluginRendererDeferred PUBLIC RENDERER_DEFERRED_DLL GLM_ENABLE_EXPERIMENTAL)
set_target_properties(PluginRendererDeferred PROPERTIES FOLDER Plugins/Renderer)

find_path(GLM_INCLUDE_DIRS "glm/glm.hpp")

target_link_libraries(
	PluginRendererDeferred
	${CORE_LIBS}
)

target_include_directories(
	PluginRendererDeferred PUBLIC
	${CODE_DIR}
	${GLM_INCLUDE_DIRS}
)

target_precompile_headers(
	PluginRendererDeferred PUBLIC
	pch.hpp
)

target_compile_definitions(PluginRendererDeferred PUBLIC NOMINMAX)
