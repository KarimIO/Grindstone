set(EDITORMODELIMPORTER_SOURCES
	ModelImporter.cpp
	ModelMaterialImporter.cpp
	${COMMON_DIR}/ResourcePipeline/AssetType.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${COMMON_DIR}/Editor/Importer.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	EntryPoint.cpp
)

set(EDITORMODELIMPORTER_HEADERS
	ModelImporter.hpp
	ModelMaterialImporter.hpp
	${COMMON_DIR}/ResourcePipeline/AssetType.hpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.hpp
	${COMMON_DIR}/ResourcePipeline/Uuid.hpp
	${COMMON_DIR}/Editor/Importer.hpp
	${ENGINE_CORE_DIR}/Utils/Utilities.hpp
	pch.hpp
)

find_package(Assimp CONFIG REQUIRED)
find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorModelImporter MODULE
	${EDITORMODELIMPORTER_SOURCES}
	${EDITORMODELIMPORTER_HEADERS}
	${CORE_UTILS}
)

set_property(TARGET PluginEditorModelImporter PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginEditorModelImporter PRIVATE cxx_std_20)
set_target_properties(PluginEditorModelImporter PROPERTIES FOLDER "Plugins/Editor/Importers")

target_compile_definitions(PluginEditorModelImporter PUBLIC EDITOR_MODEL_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorModelImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS})

target_precompile_headers(PluginEditorModelImporter PUBLIC pch.hpp)

target_link_libraries(PluginEditorModelImporter ${CORE_LIBS} assimp::assimp)
