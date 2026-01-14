set(PHYSICS_JOLT_BASE ${PLUGIN_DIR}/Grindstone.Physics.Jolt)
set(PHYSICS_JOLT_INCLUDES ${PHYSICS_JOLT_BASE}/include)
set(PHYSICS_JOLT_BIN ${PHYSICS_JOLT_BASE}/bin)
set(PHYSICS_JOLT_LIB ${PHYSICS_JOLT_BASE}/lib)
set(SRC ${PHYSICS_JOLT_BASE}/source)
set(INC ${PHYSICS_JOLT_BASE}/include)

set(JOLTPHYS_SOURCES ${SRC}/PhysicsSystem.cpp ${SRC}/PhysicsWorldContext.cpp ${SRC}/EntryPoint.cpp)
set(JOLTPHYS_HEADERS ${INC}/PhysicsSystem.hpp ${INC}/PhysicsWorldContext.hpp)

file(GLOB_RECURSE JOLTPHYS_COMPONENTS_SOURCES "${SRC}/Components/*.cpp")
file(GLOB_RECURSE JOLTPHYS_COMPONENTS_HEADER "${INC}/Components/*.hpp")
source_group("Components\\Source Files" FILES ${JOLTPHYS_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${JOLTPHYS_COMPONENTS_HEADER})

source_group("Reflection\\Source Files" FILES ${SOURCE_REFLECTION})
source_group("Reflection\\Header Files" FILES ${HEADER_REFLECTION})

find_package(imgui CONFIG REQUIRED)

add_library(
	PluginJoltPhysics MODULE
	${JOLTPHYS_SOURCES}
	${JOLTPHYS_HEADERS}
	${JOLTPHYS_COMPONENTS_SOURCES}
	${JOLTPHYS_COMPONENTS_HEADER}
	${SOURCE_REFLECTION}
	${HEADER_REFLECTION}
	${CORE_UTILS}
)

set_property(TARGET PluginJoltPhysics PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginJoltPhysics PRIVATE cxx_std_20)
set_target_properties(PluginJoltPhysics PROPERTIES FOLDER Plugins/Physics)

find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")
target_compile_definitions(PluginJoltPhysics PUBLIC "JOLT_PHYSICS;PHYSICS_DLL;GLM_ENABLE_EXPERIMENTAL")
target_compile_definitions(PluginJoltPhysics PUBLIC "$<$<CONFIG:Debug>:JPH_DEBUG_RENDERER;JPH_PROFILE_ENABLED>")
target_include_directories(PluginJoltPhysics PUBLIC ${CODE_DIR} ${ENTT_INCLUDE_DIRS} ${PLUGIN_DIR})

set_target_properties(PluginJoltPhysics PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PHYSICS_JOLT_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${PHYSICS_JOLT_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${PHYSICS_JOLT_LIB}"
)
target_precompile_headers(PluginJoltPhysics PUBLIC ${INC}/pch.hpp)

find_package(unofficial-joltphysics CONFIG REQUIRED)
target_link_libraries(PluginJoltPhysics ${CORE_LIBS} Common unofficial::joltphysics::Jolt)

add_library(PluginJoltPhysicsEditor MODULE ${SRC}/Editor.cpp ${CORE_UTILS})
set_property(TARGET PluginJoltPhysicsEditor PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginJoltPhysicsEditor PRIVATE cxx_std_20)
set_target_properties(PluginJoltPhysicsEditor PROPERTIES FOLDER Plugins/Editor)
target_include_directories(PluginJoltPhysicsEditor PUBLIC ${CODE_DIR} ${PLUGIN_DIR})
target_compile_definitions(PluginJoltPhysicsEditor PUBLIC "PHYSICS_EDITOR_DLL;GLM_ENABLE_EXPERIMENTAL")

set_target_properties(PluginJoltPhysicsEditor PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PHYSICS_JOLT_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${PHYSICS_JOLT_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${PHYSICS_JOLT_LIB}"
)
target_link_libraries(PluginJoltPhysicsEditor ${CORE_LIBS} Common EditorCommon imgui::imgui)

message(STATUS
"Grindstone.Physics.Jolt plugin provides CMake targets:

- PHYSICS_JOLT_BASE => ${PHYSICS_JOLT_BASE}
- PHYSICS_JOLT_INCLUDES => ${PHYSICS_JOLT_INCLUDES}
- PHYSICS_JOLT_BIN => ${PHYSICS_JOLT_BIN}
- PHYSICS_JOLT_LIB => ${PHYSICS_JOLT_LIB}

")
