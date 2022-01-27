set(AUDIO_OPENAL_SOURCES Core.cpp Source.cpp Clip.cpp EntryPoint.cpp)
set(AUDIO_OPENAL_HEADERS Core.hpp Source.hpp Clip.hpp)

file(GLOB_RECURSE AUDIO_OPENAL_COMPONENTS_SOURCES "Components/*.cpp")
file(GLOB_RECURSE AUDIO_OPENAL_COMPONENTS_HEADER "Components/*.hpp")
source_group("Components\\Source Files" FILES ${AUDIO_OPENAL_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${AUDIO_OPENAL_COMPONENTS_HEADER})

source_group("Reflection\\Source Files" FILES ${SOURCE_REFLECTION})
source_group("Reflection\\Header Files" FILES ${HEADER_REFLECTION})

add_library(
	AudioOpenAL MODULE
	${AUDIO_OPENAL_SOURCES}
	${AUDIO_OPENAL_HEADERS}
	${AUDIO_OPENAL_COMPONENTS_SOURCES}
	${AUDIO_OPENAL_COMPONENTS_HEADER}
	${SOURCE_REFLECTION}
	${HEADER_REFLECTION}
)

target_compile_features(AudioOpenAL PRIVATE cxx_std_17)

find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")
find_package(OpenAL CONFIG REQUIRED)
find_path(DRLIBS_INCLUDE_DIRS "dr_flac.h")
target_compile_definitions(AudioOpenAL PUBLIC AUDIO_OPENAL AUDIO_DLL)
target_include_directories(AudioOpenAL PUBLIC ${CODE_DIR} ${ENTT_INCLUDE_DIRS} ${DRLIBS_INCLUDE_DIRS})

target_precompile_headers(AudioOpenAL PUBLIC pch.hpp)

target_link_libraries(AudioOpenAL fmt::fmt OpenAL::OpenAL)
