set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(WINDOW_SOURCES Window.h win32Window.cpp Window.cpp x11Window.cpp)
include_directories(${OUTPUT_DIR}/../sources/code/Engine/)
add_library(window MODULE ${WINDOW_SOURCES})

target_compile_definitions(window PRIVATE WINDOW_DLL)