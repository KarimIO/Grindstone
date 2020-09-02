set(REN3D_SOURCES EntryPoint.cpp StaticMeshManager.cpp)
set(REN3D_HEADERS pch.hpp StaticMeshManager.hpp StaticMesh.hpp StaticMeshComponent.hpp)

add_library(PluginRenderables3D MODULE ${REN3D_SOURCES} ${REN3D_HEADERS})

target_compile_definitions(PluginRenderables3D PUBLIC RENDERABLES_3D_DLL)

target_include_directories(PluginRenderables3D
    PUBLIC ${CODE_DIR} ${DEPS_DIR}
)

target_precompile_headers(PluginRenderables3D
    PUBLIC pch.hpp)
