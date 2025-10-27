set(AI_NAVMESH_BASE ${PLUGIN_DIR}/Grindstone.Ai.NavMesh)
set(AI_NAVMESH_INCLUDES ${AI_NAVMESH_BASE}/include)
set(AI_NAVMESH_BIN ${AI_NAVMESH_BASE}/bin)
set(AI_NAVMESH_LIB ${AI_NAVMESH_BASE}/lib)
set(SRC ${AI_NAVMESH_BASE}/source)
set(INC ${AI_NAVMESH_BASE}/include)

set(AI_NAVMESH_SOURCES ${SRC}/NavMeshSystem.cpp ${SRC}/NavMeshWorldContext.cpp ${SRC}/EntryPoint.cpp)
set(AI_NAVMESH_HEADERS ${INC}/NavMeshSystem.hpp ${INC}/NavMeshWorldContext.hpp)

file(GLOB_RECURSE AI_NAVMESH_COMPONENTS_SOURCES "${SRC}/Components/*.cpp")
file(GLOB_RECURSE AI_NAVMESH_COMPONENTS_HEADER "${INC}/Components/*.hpp")
source_group("Components\\Source Files" FILES ${AI_NAVMESH_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${AI_NAVMESH_COMPONENTS_HEADER})

source_group("Reflection\\Source Files" FILES ${SOURCE_REFLECTION})
source_group("Reflection\\Header Files" FILES ${HEADER_REFLECTION})

add_library(
	PluginNavigationMesh MODULE
	${AI_NAVMESH_SOURCES}
	${AI_NAVMESH_HEADERS}
	${AI_NAVMESH_COMPONENTS_SOURCES}
	${AI_NAVMESH_COMPONENTS_HEADER}
	${SOURCE_REFLECTION}
	${HEADER_REFLECTION}
	${CORE_UTILS}
)

set_property(TARGET PluginNavigationMesh PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginNavigationMesh PRIVATE cxx_std_20)
set_target_properties(PluginNavigationMesh PROPERTIES FOLDER Plugins/Ai)

find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")
target_compile_definitions(PluginNavigationMesh PUBLIC GLM_ENABLE_EXPERIMENTAL AI_NAVMESH_DLL)
target_include_directories(PluginNavigationMesh PUBLIC ${CODE_DIR} ${ENTT_INCLUDE_DIRS} ${PLUGIN_DIR})

set_target_properties(PluginNavigationMesh PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${AI_NAVMESH_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${AI_NAVMESH_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${AI_NAVMESH_LIB}"
)
target_precompile_headers(PluginNavigationMesh PUBLIC ${INC}/pch.hpp)

find_package(recastnavigation CONFIG REQUIRED)
target_link_libraries(PluginNavigationMesh ${CORE_LIBS} Common RecastNavigation::Detour RecastNavigation::Recast RecastNavigation::DebugUtils)

message(STATUS
"PluginNavigationMesh provides CMake targets:

- AI_NAVMESH_BASE => ${AI_NAVMESH_BASE}
- AI_NAVMESH_INCLUDES => ${AI_NAVMESH_INCLUDES}
- AI_NAVMESH_BIN => ${AI_NAVMESH_BIN}
- AI_NAVMESH_LIB => ${AI_NAVMESH_LIB}

")
