set(EDITORAUDIOIMPORTER_SOURCES
	AudioImporter.cpp
	${COMMON_DIR}/ResourcePipeline/AssetType.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${COMMON_DIR}/Editor/Importer.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	EntryPoint.cpp
)

set(EDITORAUDIOIMPORTER_HEADERS
	AudioImporter.hpp
	${COMMON_DIR}/ResourcePipeline/AssetType.hpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.hpp
	${COMMON_DIR}/ResourcePipeline/Uuid.hpp
	${COMMON_DIR}/Editor/Importer.hpp
	${ENGINE_CORE_DIR}/Utils/Utilities.hpp
	pch.hpp
)

find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorAudioImporter MODULE
	${EDITORAUDIOIMPORTER_SOURCES}
	${EDITORAUDIOIMPORTER_HEADERS}
	${CORE_UTILS}
)

set_property(TARGET PluginEditorAudioImporter PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginEditorAudioImporter PRIVATE cxx_std_20)
set_target_properties(PluginEditorAudioImporter PROPERTIES FOLDER "Plugins/Editor/Importers")

target_compile_definitions(PluginEditorAudioImporter PUBLIC EDITOR_AUDIO_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorAudioImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS})

target_precompile_headers(PluginEditorAudioImporter PUBLIC pch.hpp)

target_link_libraries(PluginEditorAudioImporter ${CORE_LIBS})
