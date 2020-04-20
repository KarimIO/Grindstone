set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ENGINE_SOURCES ../Engine/Utilities/DLLHandler.hpp ../Engine/Utilities/DLLHandler.cpp)

set(GAME_EXEC_HEADERS ../ExecutableCommon/DLLEngine.hpp ../ExecutableCommon/DLLGame.hpp)
set(GAME_EXEC_SOURCES Main.cpp ../ExecutableCommon/DLLEngine.cpp ../ExecutableCommon/DLLGame.cpp)
set(GAME_EXEC_WINDOW ../WindowModule/Win32Window.cpp ../WindowModule/Win32Window.hpp ../WindowModule/BaseWindow.hpp)
source_group("Windowing" FILES ${EDITOR_WINDOW})

add_executable(GameExecutable ${GAME_EXEC_WINDOW} ${GAME_EXEC_SOURCES} ${ENGINE_SOURCES} ${GAME_EXEC_HEADERS})
target_compile_definitions(GameExecutable PRIVATE GAME_EXEC)
target_include_directories(GameExecutable PUBLIC ../ ${DEPS_DIR} ${DEPS_DIR}/spdlog/include/)