set(AUDIO_OPENAL_SOURCES Core.cpp Source.cpp AudioClipImporter.cpp EntryPoint.cpp ../../Common/ResourcePipeline/Uuid.cpp)
set(AUDIO_OPENAL_HEADERS Core.hpp Source.hpp AudioClipImporter.hpp AudioClip.hpp)

file(GLOB_RECURSE AUDIO_OPENAL_COMPONENTS_SOURCES "Components/*.cpp")
file(GLOB_RECURSE AUDIO_OPENAL_COMPONENTS_HEADER "Components/*.hpp")
source_group("Components\\Source Files" FILES ${AUDIO_OPENAL_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${AUDIO_OPENAL_COMPONENTS_HEADER})

source_group("Reflection\\Source Files" FILES ${SOURCE_REFLECTION})
source_group("Reflection\\Header Files" FILES ${HEADER_REFLECTION})

add_library(
	PluginAudioOpenAL MODULE
	${AUDIO_OPENAL_SOURCES}
	${AUDIO_OPENAL_HEADERS}
	${AUDIO_OPENAL_COMPONENTS_SOURCES}
	${AUDIO_OPENAL_COMPONENTS_HEADER}
	${SOURCE_REFLECTION}
	${HEADER_REFLECTION}
	${CORE_UTILS}
)

target_compile_features(PluginAudioOpenAL PRIVATE cxx_std_17)

find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")
find_package(OpenAL CONFIG REQUIRED)
find_path(DRLIBS_INCLUDE_DIRS "dr_flac.h")
target_compile_definitions(PluginAudioOpenAL PUBLIC AUDIO_OPENAL AUDIO_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginAudioOpenAL PUBLIC ${CODE_DIR} ${ENTT_INCLUDE_DIRS} ${DRLIBS_INCLUDE_DIRS})

target_precompile_headers(PluginAudioOpenAL PUBLIC pch.hpp)

target_link_libraries(PluginAudioOpenAL
	fmt::fmt
	OpenAL::OpenAL
	Rpcrt4.lib
)
