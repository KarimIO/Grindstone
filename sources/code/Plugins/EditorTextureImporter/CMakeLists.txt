set(EDITORTEXTUREIMPORTER_SOURCES
	TextureImporter.cpp
	${COMMON_DIR}/ResourcePipeline/AssetType.cpp
	${COMMON_DIR}/ResourcePipeline/ImporterSettings.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${COMMON_DIR}/Editor/Importer.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	EntryPoint.cpp
)

set(EDITORTEXTUREIMPORTER_HEADERS
	TextureImporter.hpp
	${COMMON_DIR}/ResourcePipeline/AssetType.hpp
	${COMMON_DIR}/ResourcePipeline/ImporterSettings.hpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.hpp
	${COMMON_DIR}/ResourcePipeline/Uuid.hpp
	${COMMON_DIR}/Editor/Importer.hpp
	${ENGINE_CORE_DIR}/Utils/Utilities.hpp
	pch.hpp
)

find_path(FMT_INCLUDE_DIRS "fmt/format.h")
find_path(STB_INCLUDE_DIRS "stb_include.h")

add_library(
	PluginEditorTextureImporter MODULE
	${EDITORTEXTUREIMPORTER_SOURCES}
	${EDITORTEXTUREIMPORTER_HEADERS}
	${CORE_UTILS}
)

set_property(TARGET PluginEditorTextureImporter PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginEditorTextureImporter PRIVATE cxx_std_20)
set_target_properties(PluginEditorTextureImporter PROPERTIES FOLDER "Plugins/Editor/Importers")

target_compile_definitions(PluginEditorTextureImporter PUBLIC EDITOR_TEXTURE_IMPORTER_DLL __STDC_LIB_EXT1__ GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorTextureImporter PUBLIC ${CODE_DIR} ${STB_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS})

target_precompile_headers(PluginEditorTextureImporter PUBLIC pch.hpp)

target_link_libraries(PluginEditorTextureImporter ${CORE_LIBS})
