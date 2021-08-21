file(GLOB_RECURSE OPENAL_COMPONENTS_SOURCES "Components/*.cpp")
file(GLOB_RECURSE OPENAL_COMPONENTS_HEADERS "Components/*.hpp")
source_group("Components\\Source Files" FILES ${OPENAL_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${OPENAL_COMPONENTS_HEADERS})

set(OPENAL_MAIN_SOURCES Core.cpp Entrypoint.cpp)
set(OPENAL_MAIN_HEADERS Core.hpp pch.hpp)

set(OPENAL_SOURCES ${OPENAL_MAIN_SOURCES} ${OPENAL_COMPONENTS_SOURCES})
set(OPENAL_HEADERS ${OPENAL_MAIN_HEADERS} ${OPENAL_COMPONENTS_HEADERS})

add_library(PluginAudioOpenAL MODULE ${OPENAL_SOURCES} ${OPENAL_HEADERS})

target_include_directories(PluginAudioOpenAL PUBLIC ${GLM_DIR} ${DEPS_DIR})

target_compile_features(PluginAudioOpenAL PRIVATE cxx_std_17)

target_compile_definitions(PluginAudioOpenAL PUBLIC AUDIO_OPENAL AUDIO_DLL)

target_include_directories(PluginAudioOpenAL
	PUBLIC
	${CODE_DIR}
	${DEPS_DIR}
	${DEPS_DIR}/entt/src
)

target_precompile_headers(PluginAudioOpenAL PUBLIC pch.hpp )

find_package(OpenAL) 
target_link_libraries(PluginAudioOpenAL ${OPENAL_LIBRARY})
