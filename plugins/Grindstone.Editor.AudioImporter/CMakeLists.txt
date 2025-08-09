set(EDITOR_AUDIO_IMPORTER_BASE ${PLUGIN_DIR}/Grindstone.Editor.AudioImporter)
set(EDITOR_AUDIO_IMPORTER_INCLUDES ${EDITOR_AUDIO_IMPORTER_BASE}/include)
set(EDITOR_AUDIO_IMPORTER_BIN ${EDITOR_AUDIO_IMPORTER_BASE}/bin)
set(EDITOR_AUDIO_IMPORTER_LIB ${EDITOR_AUDIO_IMPORTER_BASE}/lib)
set(SRC ${EDITOR_AUDIO_IMPORTER_BASE}/source)
set(INC ${EDITOR_AUDIO_IMPORTER_BASE}/include)

set(EDITORAUDIOIMPORTER_SOURCES
	${SRC}/AudioImporter.cpp
	${COMMON_DIR}/ResourcePipeline/AssetType.cpp
	${COMMON_DIR}/ResourcePipeline/ImporterSettings.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${EDITOR_DIR}/EditorManagerInstance.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	${SRC}/EntryPoint.cpp
)

set(EDITORAUDIOIMPORTER_HEADERS
	${INC}/AudioImporter.hpp
	${COMMON_DIR}/ResourcePipeline/AssetType.hpp
	${COMMON_DIR}/ResourcePipeline/ImporterSettings.hpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.hpp
	${COMMON_DIR}/ResourcePipeline/Uuid.hpp
	${COMMON_DIR}/Editor/Importer.hpp
	${ENGINE_CORE_DIR}/Utils/Utilities.hpp
	${INC}/pch.hpp
)

find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorAudioImporter MODULE
	${EDITORAUDIOIMPORTER_SOURCES}
	${EDITORAUDIOIMPORTER_HEADERS}
	${CORE_UTILS}
)

set_target_properties(PluginEditorAudioImporter PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${EDITOR_AUDIO_IMPORTER_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${EDITOR_AUDIO_IMPORTER_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_AUDIO_IMPORTER_LIB}"
)
set_property(TARGET PluginEditorAudioImporter PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginEditorAudioImporter PRIVATE cxx_std_20)
set_target_properties(PluginEditorAudioImporter PROPERTIES FOLDER "Plugins/Editor/Importers")

target_compile_definitions(PluginEditorAudioImporter PUBLIC EDITOR_AUDIO_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorAudioImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS} ${PLUGIN_DIR})

target_precompile_headers(PluginEditorAudioImporter PUBLIC ${INC}/pch.hpp)

target_link_libraries(PluginEditorAudioImporter ${CORE_LIBS})

message(STATUS
"PluginEditorAudioImporter provides CMake targets:

- EDITOR_AUDIO_IMPORTER_BASE => ${EDITOR_AUDIO_IMPORTER_BASE}
- EDITOR_AUDIO_IMPORTER_INCLUDES => ${EDITOR_AUDIO_IMPORTER_INCLUDES}
- EDITOR_AUDIO_IMPORTER_BIN => ${EDITOR_AUDIO_IMPORTER_BIN}
- EDITOR_AUDIO_IMPORTER_LIB => ${EDITOR_AUDIO_IMPORTER_LIB}


")
