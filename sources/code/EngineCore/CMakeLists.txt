set(ENGINE_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCE_ECS "${ENGINE_CORE_DIR}/ECS/*.cpp")
file(GLOB_RECURSE HEADER_ECS "${ENGINE_CORE_DIR}/ECS/*.hpp")
source_group("ECS\\Source Files" FILES ${SOURCE_ECS})
source_group("ECS\\Header Files" FILES ${HEADER_ECS})

file(GLOB_RECURSE SOURCE_SCENES "${ENGINE_CORE_DIR}/Scenes/*.cpp")
file(GLOB_RECURSE HEADER_SCENES "${ENGINE_CORE_DIR}/Scenes/*.hpp")
source_group("Scenes\\Source Files" FILES ${SOURCE_SCENES})
source_group("Scenes\\Header Files" FILES ${HEADER_SCENES})

file(GLOB_RECURSE SOURCE_PLUGIN_SYSTEM "${ENGINE_CORE_DIR}/PluginSystem/*.cpp")
file(GLOB_RECURSE HEADER_PLUGIN_SYSTEM "${ENGINE_CORE_DIR}/PluginSystem/*.hpp")
source_group("Plugin System\\Source Files" FILES ${SOURCE_PLUGIN_SYSTEM})
source_group("Plugin System\\Header Files" FILES ${HEADER_PLUGIN_SYSTEM})

file(GLOB_RECURSE SOURCE_COMMON_UTILS ../Common/Utilities/ModuleLoading.cpp)
file(GLOB_RECURSE HEADER_COMMON_UTILS ../Common/Utilities/ModuleLoading.hpp)
source_group("Common\\Utilities\\Source Files" FILES ${SOURCE_COMMON_UTILS})
source_group("Common\\Utilities\\Header Files" FILES ${HEADER_COMMON_UTILS})

file(GLOB_RECURSE SOURCE_MAIN EngineCore.cpp EntryPoint.cpp Logger.cpp Profiling.cpp)
file(GLOB_RECURSE HEADER_MAIN EngineCore.hpp pch.hpp Logger.hpp Profiling.hpp)

set(ENGINE_CORE_SOURCES ${SOURCE_MAIN} ${SOURCE_PLUGIN_SYSTEM} ${SOURCE_ECS} ${SOURCE_COMMON_UTILS} ${SOURCE_SCENES})
set(ENGINE_CORE_HEADERS ${HEADER_MAIN} ${HEADER_PLUGIN_SYSTEM} ${HEADER_ECS} ${HEADER_COMMON_UTILS} ${HEADER_SCENES})

add_library(EngineCore MODULE ${ENGINE_CORE_SOURCES} ${ENGINE_CORE_HEADERS})

target_compile_features(EngineCore PRIVATE cxx_std_17)

target_compile_definitions(EngineCore PUBLIC ENGINE_CORE)

target_include_directories(EngineCore
    PUBLIC ../ ${DEPS_DIR} ${DEPS_DIR}/spdlog/include/
)

target_precompile_headers(EngineCore
    PUBLIC pch.hpp)