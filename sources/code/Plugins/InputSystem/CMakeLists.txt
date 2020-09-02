set(INPUT_SOURCES EntryPoint.cpp InputSystem.cpp)
set(INPUT_HEADERS pch.hpp InputSystem.hpp)

add_library(PluginInputSystem MODULE ${INPUT_SOURCES} ${INPUT_HEADERS})

target_compile_definitions(PluginInputSystem PUBLIC INPUT_SYSTEM_DLL)

target_include_directories(PluginInputSystem
    PUBLIC ${CODE_DIR} ${DEPS_DIR}
)

target_precompile_headers(PluginInputSystem
    PUBLIC pch.hpp)
