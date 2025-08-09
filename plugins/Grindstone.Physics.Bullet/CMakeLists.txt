set(PHYSICS_BULLET_BASE ${PLUGIN_DIR}/Grindstone.Physics.Bullet)
set(PHYSICS_BULLET_INCLUDES ${PHYSICS_BULLET_BASE}/include)
set(PHYSICS_BULLET_BIN ${PHYSICS_BULLET_BASE}/bin)
set(PHYSICS_BULLET_LIB ${PHYSICS_BULLET_BASE}/lib)
set(SRC ${PHYSICS_BULLET_BASE}/source)
set(INC ${PHYSICS_BULLET_BASE}/include)

set(BULLETPHYS_SOURCES ${SRC}/PhysicsSystem.cpp ${SRC}/PhysicsWorldContext.cpp ${SRC}/EntryPoint.cpp)
set(BULLETPHYS_HEADERS ${INC}/PhysicsSystem.hpp ${INC}/PhysicsWorldContext.hpp)

file(GLOB_RECURSE BULLETPHYS_COMPONENTS_SOURCES "${SRC}/Components/*.cpp")
file(GLOB_RECURSE BULLETPHYS_COMPONENTS_HEADER "${INC}/Components/*.hpp")
source_group("Components\\Source Files" FILES ${BULLETPHYS_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${BULLETPHYS_COMPONENTS_HEADER})

source_group("Reflection\\Source Files" FILES ${SOURCE_REFLECTION})
source_group("Reflection\\Header Files" FILES ${HEADER_REFLECTION})

add_library(
	PluginBulletPhysics MODULE
	${BULLETPHYS_SOURCES}
	${BULLETPHYS_HEADERS}
	${BULLETPHYS_COMPONENTS_SOURCES}
	${BULLETPHYS_COMPONENTS_HEADER}
	${SOURCE_REFLECTION}
	${HEADER_REFLECTION}
	${CORE_UTILS}
)

set_property(TARGET PluginBulletPhysics PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginBulletPhysics PRIVATE cxx_std_20)
set_target_properties(PluginBulletPhysics PROPERTIES FOLDER Plugins/Physics)

find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")
target_compile_definitions(PluginBulletPhysics PUBLIC BULLET_PHYSICS PHYSICS_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginBulletPhysics PUBLIC ${CODE_DIR} ${ENTT_INCLUDE_DIRS} ${PLUGIN_DIR})

set_target_properties(PluginBulletPhysics PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PHYSICS_BULLET_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${PHYSICS_BULLET_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${PHYSICS_BULLET_LIB}"
)
target_precompile_headers(PluginBulletPhysics PUBLIC ${INC}/pch.hpp)

find_package(Bullet CONFIG REQUIRED)
target_link_libraries(PluginBulletPhysics ${CORE_LIBS} BulletSoftBody BulletDynamics BulletCollision Bullet3Common LinearMath)

message(STATUS
"PluginBulletPhysics provides CMake targets:

- PHYSICS_BULLET_BASE => ${PHYSICS_BULLET_BASE}
- PHYSICS_BULLET_INCLUDES => ${PHYSICS_BULLET_INCLUDES}
- PHYSICS_BULLET_BIN => ${PHYSICS_BULLET_BIN}
- PHYSICS_BULLET_LIB => ${PHYSICS_BULLET_LIB}


")
