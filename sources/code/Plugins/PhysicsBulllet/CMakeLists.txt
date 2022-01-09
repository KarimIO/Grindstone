set(BULLETPHYS_SOURCES PhysicsSystem.cpp EntryPoint.cpp)
set(BULLETPHYS_HEADERS PhysicsSystem.hpp)

file(GLOB_RECURSE BULLETPHYS_COMPONENTS_SOURCES "Components/*.cpp")
file(GLOB_RECURSE BULLETPHYS_COMPONENTS_HEADER "Components/*.hpp")
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
)

target_compile_features(PluginBulletPhysics PRIVATE cxx_std_17)

find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")
target_compile_definitions(PluginBulletPhysics PUBLIC BULLET_PHYSICS PHYSICS_DLL)
target_include_directories(PluginBulletPhysics PUBLIC ${CODE_DIR} ${ENTT_INCLUDE_DIRS})

target_precompile_headers(PluginBulletPhysics
    PUBLIC pch.hpp
)


find_package(Bullet CONFIG REQUIRED)
target_link_libraries(PluginBulletPhysics BulletSoftBody BulletDynamics BulletCollision Bullet3Common LinearMath)
