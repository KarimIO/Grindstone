set(CAM_SOURCES EntryPoint.cpp RenderCameras.cpp)
set(CAM_HEADERS pch.hpp Camera.hpp RenderCameras.hpp)

add_library(PluginCamera MODULE ${CAM_SOURCES} ${CAM_HEADERS})

target_compile_definitions(PluginCamera PUBLIC CAMERA_DLL)

target_include_directories(PluginCamera
    PUBLIC ${CODE_DIR} ${DEPS_DIR}
)

target_precompile_headers(PluginCamera
    PUBLIC pch.hpp)
