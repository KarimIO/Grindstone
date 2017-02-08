set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUDIO_SOURCES Audio.cpp SoundFileSDL.cpp)

SET(SDL_DIR "D:/Libraries/SDL" CACHE PATH "Location of SDL Library")
include_directories(${SDL_DIR}/include)
link_directories(${SDL_DIR}/lib/x64)

add_library(audiosdl MODULE ${AUDIO_SOURCES})

target_compile_definitions(audiosdl PRIVATE AUDIO_DLL)

target_link_libraries(audiosdl SDL2 SDL2main SDL2_mixer)