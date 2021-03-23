set(INPUT_SOURCES EntryPoint.cpp InputManager.cpp)
set(INPUT_HEADERS pch.hpp InputManager.hpp)

add_library(PluginInputSystem MODULE ${INPUT_SOURCES} ${INPUT_HEADERS})

target_compile_definitions(PluginInputSystem PUBLIC INPUT_SYSTEM_DLL)

target_include_directories(PluginInputSystem
    PUBLIC
    ${CODE_DIR}
    ${DEPS_DIR}
    ${DEPS_DIR}/entt/src
)

target_precompile_headers(PluginInputSystem
    PUBLIC
    pch.hpp
)
