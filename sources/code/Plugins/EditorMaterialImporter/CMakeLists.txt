set(EDITORMATERIALIMPORTER_SOURCES
	MaterialImporter.cpp
	${COMMON_DIR}/ResourcePipeline/AssetType.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${COMMON_DIR}/Editor/Importer.cpp
	${ENGINECORE_DIR}/Utils/Utilities.cpp
	EntryPoint.cpp
)

set(EDITORMATERIALIMPORTER_HEADERS
	MaterialImporter.hpp 
	${COMMON_DIR}/ResourcePipeline/AssetType.hpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.hpp
	${COMMON_DIR}/ResourcePipeline/Uuid.hpp
	${COMMON_DIR}/Editor/Importer.hpp
	${ENGINECORE_DIR}/Utils/Utilities.hpp
	pch.hpp
)

find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorMaterialImporter MODULE
	${EDITORMATERIALIMPORTER_SOURCES}
	${EDITORMATERIALIMPORTER_HEADERS}
	${CORE_UTILS}
)

target_compile_features(PluginEditorMaterialImporter PRIVATE cxx_std_17)

target_compile_definitions(PluginEditorMaterialImporter PUBLIC EDITOR_MATERIAL_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorMaterialImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS})

target_precompile_headers(PluginEditorMaterialImporter PUBLIC pch.hpp)

target_link_libraries(PluginEditorMaterialImporter ${CORE_LIBS} Rpcrt4.lib)
