set(AUDIO_OPENAL_BASE ${PLUGIN_DIR}/Grindstone.Audio.OpenAL)
set(AUDIO_OPENAL_INCLUDES ${AUDIO_OPENAL_BASE}/include)
set(AUDIO_OPENAL_BIN ${AUDIO_OPENAL_BASE}/bin)
set(AUDIO_OPENAL_LIB ${AUDIO_OPENAL_BASE}/lib)
set(SRC ${AUDIO_OPENAL_BASE}/source)
set(INC ${AUDIO_OPENAL_BASE}/include)

set(AUDIO_OPENAL_SOURCES ${SRC}/Core.cpp ${SRC}/Source.cpp ${SRC}/AudioClipImporter.cpp ${SRC}/EntryPoint.cpp ${COMMON_DIR}/ResourcePipeline/Uuid.cpp)
set(AUDIO_OPENAL_HEADERS ${INC}/Core.hpp ${INC}/Source.hpp ${INC}/AudioClipImporter.hpp ${INC}/AudioClip.hpp)

file(GLOB_RECURSE AUDIO_OPENAL_COMPONENTS_SOURCES "${SRC}/Components/*.cpp")
file(GLOB_RECURSE AUDIO_OPENAL_COMPONENTS_HEADER "${INC}/Components/*.hpp")
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

set_property(TARGET PluginAudioOpenAL PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginAudioOpenAL PRIVATE cxx_std_20)
set_target_properties(PluginAudioOpenAL PROPERTIES FOLDER "Plugins/Audio")

set_target_properties(PluginAudioOpenAL PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${AUDIO_OPENAL_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${AUDIO_OPENAL_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${AUDIO_OPENAL_LIB}"
)
find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")
find_package(OpenAL CONFIG REQUIRED)
find_path(DRLIBS_INCLUDE_DIRS "dr_flac.h")
target_compile_definitions(PluginAudioOpenAL PUBLIC AUDIO_OPENAL AUDIO_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginAudioOpenAL PUBLIC ${CODE_DIR} ${ENTT_INCLUDE_DIRS} ${DRLIBS_INCLUDE_DIRS} ${PLUGIN_DIR})

target_precompile_headers(PluginAudioOpenAL PUBLIC ${INC}/pch.hpp)

target_link_libraries(PluginAudioOpenAL
	${CORE_LIBS}
	OpenAL::OpenAL
	Common
)

message(STATUS
"PluginAudioOpenAL provides CMake targets:

- AUDIO_OPENAL_BASE => ${AUDIO_OPENAL_BASE}
- AUDIO_OPENAL_INCLUDES => ${AUDIO_OPENAL_INCLUDES}
- AUDIO_OPENAL_BIN => ${AUDIO_OPENAL_BIN}
- AUDIO_OPENAL_LIB => ${AUDIO_OPENAL_LIB}

")
