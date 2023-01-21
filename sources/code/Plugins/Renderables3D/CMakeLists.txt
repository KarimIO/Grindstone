set(RENDERABLES_3D_SOURCES Mesh3dRenderer.cpp EntryPoint.cpp ../../EngineCore/ECS/Entity.cpp ../../Common/ResourcePipeline/Uuid.hpp)
set(RENDERABLES_3D_HEADERS Mesh3dRenderer.hpp)

file(GLOB_RECURSE RENDERABLES_3D_ASSETS_SOURCES "Assets/*.cpp")
file(GLOB_RECURSE RENDERABLES_3D_ASSETS_HEADER "Assets/*.hpp")
source_group("Assets\\Source Files" FILES ${RENDERABLES_3D_ASSETS_SOURCES})
source_group("Assets\\Header Files" FILES ${RENDERABLES_3D_ASSETS_HEADER})

file(GLOB_RECURSE RENDERABLES_3D_COMPONENTS_SOURCES "Components/*.cpp")
file(GLOB_RECURSE RENDERABLES_3D_COMPONENTS_HEADER "Components/*.hpp")
source_group("Components\\Source Files" FILES ${RENDERABLES_3D_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${RENDERABLES_3D_COMPONENTS_HEADER})

add_library(
	PluginRenderables3D
	MODULE
	${RENDERABLES_3D_SOURCES}
	${RENDERABLES_3D_HEADER}
	${RENDERABLES_3D_ASSETS_SOURCES}
	${RENDERABLES_3D_ASSETS_HEADER}
	${RENDERABLES_3D_COMPONENTS_SOURCES}
	${RENDERABLES_3D_COMPONENTS_HEADER}
)

target_compile_definitions(PluginRenderables3D PUBLIC RENDERABLES_3D_DLL)

find_path(GLM_INCLUDE_DIRS "glm/glm.hpp")

target_include_directories(PluginRenderables3D
    PUBLIC ${CODE_DIR} ${GLM_INCLUDE_DIRS}
)

target_precompile_headers(PluginRenderables3D
    PUBLIC pch.hpp)
