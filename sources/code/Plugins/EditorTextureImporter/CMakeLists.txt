set(EDITORTEXTUREIMPORTER_SOURCES TextureImporter.cpp EntryPoint.cpp)
set(EDITORTEXTUREIMPORTER_HEADERS TextureImporter.hpp pch.hpp bc6h_enc.h)

find_path(FMT_INCLUDE_DIRS "fmt/format.h")
find_path(STB_INCLUDE_DIRS "stb_include.h")

add_library(
	PluginEditorTextureImporter MODULE
	${EDITORTEXTUREIMPORTER_SOURCES}
	${EDITORTEXTUREIMPORTER_HEADERS}
)


target_compile_features(PluginEditorTextureImporter PRIVATE cxx_std_17)

target_compile_definitions(PluginEditorTextureImporter PUBLIC EDITOR_TEXTURE_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorTextureImporter PUBLIC ${CODE_DIR} ${STB_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS})

target_precompile_headers(PluginEditorTextureImporter PUBLIC pch.hpp)
