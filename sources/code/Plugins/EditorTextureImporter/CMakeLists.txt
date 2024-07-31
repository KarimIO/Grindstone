set(EDITORTEXTUREIMPORTER_SOURCES
	TextureImporter.cpp
	${COMMON_DIR}/ResourcePipeline/AssetType.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${COMMON_DIR}/Editor/Importer.cpp
	${ENGINECORE_DIR}/Utils/Utilities.cpp
	EntryPoint.cpp
)

set(EDITORTEXTUREIMPORTER_HEADERS
	TextureImporter.hpp
	${COMMON_DIR}/ResourcePipeline/AssetType.hpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.hpp
	${COMMON_DIR}/ResourcePipeline/Uuid.hpp
	${COMMON_DIR}/Editor/Importer.hpp
	${ENGINECORE_DIR}/Utils/Utilities.hpp
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

target_compile_features(PluginEditorTextureImporter PRIVATE cxx_std_17)

target_compile_definitions(PluginEditorTextureImporter PUBLIC EDITOR_TEXTURE_IMPORTER_DLL __STDC_LIB_EXT1__ GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorTextureImporter PUBLIC ${CODE_DIR} ${STB_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS})

target_precompile_headers(PluginEditorTextureImporter PUBLIC pch.hpp)

target_link_libraries(PluginEditorTextureImporter ${CORE_LIBS} Rpcrt4.lib)
