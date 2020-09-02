set(MAT_SOURCES EntryPoint.cpp MaterialManager.cpp)
set(MAT_HEADERS pch.hpp MaterialManager.hpp Material.hpp Pipeline.hpp RenderQueue.hpp)

add_library(PluginMaterials MODULE ${MAT_SOURCES} ${MAT_HEADERS})

target_compile_definitions(PluginMaterials PUBLIC RENDERER_MATERIAL_DLL)

target_include_directories(PluginMaterials
    PUBLIC ${CODE_DIR} ${DEPS_DIR}
)

target_precompile_headers(PluginMaterials
    PUBLIC pch.hpp)
