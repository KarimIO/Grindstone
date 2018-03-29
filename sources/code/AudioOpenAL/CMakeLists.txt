set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUDIO_SOURCES OpenALWrapper.cpp OpenALBuffer.cpp OpenALSource.cpp)

add_library(openal MODULE ${AUDIO_SOURCES})

target_compile_definitions(openal PRIVATE AUDIO_DLL)

target_link_libraries(openal ${OPENAL_LIBRARY})
target_include_directories(openal PUBLIC ${OPENAL_INCLUDE_DIR})