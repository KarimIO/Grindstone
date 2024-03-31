set(EDITORMATERIALIMPORTER_SOURCES MaterialImporter.cpp EntryPoint.cpp)
set(EDITORMATERIALIMPORTER_HEADERS MaterialImporter.hpp pch.hpp)

find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorMaterialImporter MODULE
	${EDITORMATERIALIMPORTER_SOURCES}
	${EDITORMATERIALIMPORTER_HEADERS}
)

target_compile_features(PluginEditorMaterialImporter PRIVATE cxx_std_17)

target_compile_definitions(PluginEditorMaterialImporter PUBLIC EDITOR_MATERIAL_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorMaterialImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS})

target_precompile_headers(PluginEditorMaterialImporter PUBLIC pch.hpp)
