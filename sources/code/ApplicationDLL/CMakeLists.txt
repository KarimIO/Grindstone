set(ENGINE_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCE_MAIN Application.cpp)
file(GLOB_RECURSE HEADER_MAIN pch.hpp)

set(ENGINE_CORE_SOURCES ${SOURCE_MAIN})
set(ENGINE_CORE_HEADERS ${HEADER_MAIN})

add_library(ApplicationDLL MODULE ${ENGINE_CORE_SOURCES} ${ENGINE_CORE_HEADERS})

target_compile_definitions(ApplicationDLL PUBLIC APP_DLL)

target_include_directories(ApplicationDLL PUBLIC ${CODE_DIR} ${VCPKG_INCLUDE_DIR})

target_precompile_headers(ApplicationDLL PUBLIC pch.hpp)
