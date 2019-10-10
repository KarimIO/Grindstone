set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ENGINE_SOURCES Game.hpp Game.cpp TestSystem.hpp TestSystem.cpp)

set(GAME_HEADERS Game.hpp TestSystem.hpp)
set(GAME_SOURCES Game.cpp TestSystem.cpp)

add_library(game MODULE ${GAME_SOURCES} ${GAME_HEADERS} ${GAME_COMMON})
target_compile_definitions(game PRIVATE GAME_DLL)