set(RENDERABLES_3D_BASE ${PLUGIN_DIR}/Grindstone.Renderables.3D)
set(RENDERABLES_3D_INCLUDES ${RENDERABLES_3D_BASE}/include)
set(RENDERABLES_3D_BIN ${RENDERABLES_3D_BASE}/bin)
set(RENDERABLES_3D_LIB ${RENDERABLES_3D_BASE}/lib)
set(SRC ${RENDERABLES_3D_BASE}/source)
set(INC ${RENDERABLES_3D_BASE}/include)

set(RENDERABLES_3D_SOURCES ${SRC}/Mesh3dRenderer.cpp ${SRC}/EntryPoint.cpp ${COMMON_DIR}/ResourcePipeline/Uuid.cpp ${COMMON_DIR}/HashedString.cpp ${ENGINE_CORE_DIR}/Reflection/PrintReflectionData.cpp)
set(RENDERABLES_3D_HEADERS ${INC}/Mesh3dRenderer.hpp)

file(GLOB_RECURSE RENDERABLES_3D_ASSETS_SOURCES "${SRC}/Assets/*.cpp")
file(GLOB_RECURSE RENDERABLES_3D_ASSETS_HEADER "${INC}/Assets/*.hpp")
source_group("Assets\\Source Files" FILES ${RENDERABLES_3D_ASSETS_SOURCES})
source_group("Assets\\Header Files" FILES ${RENDERABLES_3D_ASSETS_HEADER})

file(GLOB_RECURSE RENDERABLES_3D_COMPONENTS_SOURCES "${SRC}/Components/*.cpp")
file(GLOB_RECURSE RENDERABLES_3D_COMPONENTS_HEADER "${INC}/Components/*.hpp")
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
	${CORE_UTILS}
)

set_property(TARGET PluginRenderables3D PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_definitions(PluginRenderables3D PUBLIC RENDERABLES_3D_DLL GLM_ENABLE_EXPERIMENTAL)
set_target_properties(PluginRenderables3D PROPERTIES FOLDER Plugins/Renderables)

find_path(GLM_INCLUDE_DIRS "glm/glm.hpp")

set_target_properties(PluginRenderables3D PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${RENDERABLES_3D_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${RENDERABLES_3D_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${RENDERABLES_3D_LIB}"
)
target_link_libraries(PluginRenderables3D ${CORE_LIBS} Common)

target_include_directories(
	PluginRenderables3D PUBLIC
	${CODE_DIR}
	${GLM_INCLUDE_DIRS}
	${PLUGIN_DIR}
)

target_precompile_headers(
	PluginRenderables3D PUBLIC
	${INC}/pch.hpp
)

message(STATUS
"PluginRenderables3D provides CMake targets:

- RENDERABLES_3D_BASE => ${RENDERABLES_3D_BASE}
- RENDERABLES_3D_INCLUDES => ${RENDERABLES_3D_INCLUDES}
- RENDERABLES_3D_BIN => ${RENDERABLES_3D_BIN}
- RENDERABLES_3D_LIB => ${RENDERABLES_3D_LIB}

")
