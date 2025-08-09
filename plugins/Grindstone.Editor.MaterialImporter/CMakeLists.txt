set(EDITOR_MATERIAL_IMPORTER_BASE ${PLUGIN_DIR}/Grindstone.Editor.MaterialImporter)
set(EDITOR_MATERIAL_IMPORTER_INCLUDES ${EDITOR_MATERIAL_IMPORTER_BASE}/include)
set(EDITOR_MATERIAL_IMPORTER_BIN ${EDITOR_MATERIAL_IMPORTER_BASE}/bin)
set(EDITOR_MATERIAL_IMPORTER_LIB ${EDITOR_MATERIAL_IMPORTER_BASE}/lib)
set(SRC ${EDITOR_MATERIAL_IMPORTER_BASE}/source)
set(INC ${EDITOR_MATERIAL_IMPORTER_BASE}/include)

set(EDITORMATERIALIMPORTER_SOURCES
	${SRC}/MaterialImporter.cpp
	${COMMON_DIR}/ResourcePipeline/AssetType.cpp
	${COMMON_DIR}/ResourcePipeline/ImporterSettings.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${EDITOR_DIR}/EditorManagerInstance.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	${SRC}/EntryPoint.cpp
)

set(EDITORMATERIALIMPORTER_HEADERS
	${INC}/MaterialImporter.hpp
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
	PluginEditorMaterialImporter MODULE
	${EDITORMATERIALIMPORTER_SOURCES}
	${EDITORMATERIALIMPORTER_HEADERS}
	${CORE_UTILS}
)

set_target_properties(PluginEditorMaterialImporter PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${EDITOR_MATERIAL_IMPORTER_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${EDITOR_MATERIAL_IMPORTER_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_MATERIAL_IMPORTER_LIB}"
)
set_property(TARGET PluginEditorMaterialImporter PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginEditorMaterialImporter PRIVATE cxx_std_20)
set_target_properties(PluginEditorMaterialImporter PROPERTIES FOLDER "Plugins/Editor/Importers")

target_compile_definitions(PluginEditorMaterialImporter PUBLIC EDITOR_MATERIAL_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorMaterialImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS} ${PLUGIN_DIR})

target_precompile_headers(PluginEditorMaterialImporter PUBLIC ${INC}/pch.hpp)

target_link_libraries(PluginEditorMaterialImporter ${CORE_LIBS})

message(STATUS
"PluginEditorMaterialImporter provides CMake targets:

- EDITOR_MATERIAL_IMPORTER_BASE => ${EDITOR_MATERIAL_IMPORTER_BASE}
- EDITOR_MATERIAL_IMPORTER_INCLUDES => ${EDITOR_MATERIAL_IMPORTER_INCLUDES}
- EDITOR_MATERIAL_IMPORTER_BIN => ${EDITOR_MATERIAL_IMPORTER_BIN}
- EDITOR_MATERIAL_IMPORTER_LIB => ${EDITOR_MATERIAL_IMPORTER_LIB}


")
