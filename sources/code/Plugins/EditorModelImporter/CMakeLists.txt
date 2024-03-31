set(EDITORMODELIMPORTER_SOURCES ModelImporter.cpp EntryPoint.cpp)
set(EDITORMODELIMPORTER_HEADERS ModelImporter.hpp pch.hpp)

find_package(Assimp CONFIG REQUIRED)
find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorModelImporter MODULE
	${EDITORMODELIMPORTER_SOURCES}
	${EDITORMODELIMPORTER_HEADERS}
)

target_compile_features(PluginEditorModelImporter PRIVATE cxx_std_17)

target_compile_definitions(PluginEditorModelImporter PUBLIC EDITOR_MODEL_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorModelImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS})

target_precompile_headers(PluginEditorModelImporter PUBLIC pch.hpp)

target_link_libraries(PluginEditorModelImporter assimp::assimp)
