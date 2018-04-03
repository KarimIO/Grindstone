set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUDIO_COMMON ../AudioCommon/AudioCommon.hpp ../AudioCommon/SoundBuffer.hpp ../AudioCommon/SoundSource.hpp)
set(AUDIO_HEADERS OpenALWrapper.hpp OpenALBuffer.hpp OpenALSource.hpp)
set(AUDIO_SOURCES OpenALWrapper.cpp OpenALBuffer.cpp OpenALSource.cpp)

add_library(audioopenal MODULE ${AUDIO_SOURCES} ${AUDIO_HEADERS} ${AUDIO_COMMON})

target_compile_definitions(audioopenal PRIVATE AUDIO_DLL)

target_link_libraries(audioopenal ${OPENAL_LIBRARY})
target_include_directories(audioopenal PUBLIC ${OPENAL_INCLUDE_DIR})