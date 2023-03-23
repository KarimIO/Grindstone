set(ANIMATOR_SOURCES AnimationSystem.cpp EntryPoint.cpp)
set(ANIMATOR_HEADERS AnimationSystem.hpps)

file(GLOB_RECURSE ANIMATOR_ASSETS_SOURCES "Assets/*.cpp")
file(GLOB_RECURSE ANIMATOR_ASSETS_HEADER "Assets/*.hpp")
source_group("Assets\\Source Files" FILES ${ANIMATOR_ASSETS_SOURCES})
source_group("Assets\\Header Files" FILES ${ANIMATOR_ASSETS_HEADER})

file(GLOB_RECURSE ANIMATOR_COMPONENTS_SOURCES "Components/*.cpp")
file(GLOB_RECURSE ANIMATOR_COMPONENTS_HEADER "Components/*.hpp")
source_group("Components\\Source Files" FILES ${ANIMATOR_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${ANIMATOR_COMPONENTS_HEADER})

add_library(
	PluginAnimator
	MODULE
	${ANIMATOR_SOURCES}
	${ANIMATOR_HEADER}
	${ANIMATOR_ASSETS_SOURCES}
	${ANIMATOR_ASSETS_HEADER}
	${ANIMATOR_COMPONENTS_SOURCES}
	${ANIMATOR_COMPONENTS_HEADER}
)

target_compile_definitions(PluginAnimator PUBLIC ANIMATOR_DLL)

target_include_directories(PluginAnimator
    PUBLIC ${CODE_DIR} ${DEPS_DIR}
)

target_precompile_headers(PluginAnimator
    PUBLIC pch.hpp)
