set(RENDERPATH_DEFERRED_SOURCES EntryPoint.cpp RenderPathDeferred.cpp)
set(RENDERPATH_DEFERRED_HEADERS pch.hpp RenderPathDeferred.hpp)

add_library(PluginRenderPathDeferred MODULE ${RENDERPATH_DEFERRED_SOURCES} ${RENDERPATH_DEFERRED_HEADERS})

target_compile_definitions(PluginRenderPathDeferred PUBLIC RENDERPATH_DEFERRED_DLL)

target_include_directories(PluginRenderPathDeferred
    PUBLIC ${CODE_DIR} ${DEPS_DIR}
)

target_precompile_headers(PluginRenderPathDeferred
    PUBLIC pch.hpp)
