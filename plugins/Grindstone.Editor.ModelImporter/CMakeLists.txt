set(EDITOR_MODEL_IMPORTER_BASE ${PLUGIN_DIR}/Grindstone.Editor.ModelImporter)
set(EDITOR_MODEL_IMPORTER_INCLUDES ${EDITOR_MODEL_IMPORTER_BASE}/include)
set(EDITOR_MODEL_IMPORTER_BIN ${EDITOR_MODEL_IMPORTER_BASE}/bin)
set(EDITOR_MODEL_IMPORTER_LIB ${EDITOR_MODEL_IMPORTER_BASE}/lib)
set(SRC ${EDITOR_MODEL_IMPORTER_BASE}/source)
set(INC ${EDITOR_MODEL_IMPORTER_BASE}/include)

set(EDITORMODELIMPORTER_SOURCES
	${SRC}/ModelImporter.cpp
	${SRC}/ModelMaterialImporter.cpp



	${EDITOR_DIR}/EditorManagerInstance.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	${SRC}/EntryPoint.cpp
)

set(EDITORMODELIMPORTER_HEADERS
	${INC}/ModelImporter.hpp
	${INC}/ModelMaterialImporter.hpp



	${ENGINE_CORE_DIR}/Utils/Utilities.hpp
	${INC}/pch.hpp
)

find_package(Assimp CONFIG REQUIRED)
find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorModelImporter MODULE
	${EDITORMODELIMPORTER_SOURCES}
	${EDITORMODELIMPORTER_HEADERS}
	${CORE_UTILS}
)

find_package(imgui CONFIG REQUIRED)

set_target_properties(PluginEditorModelImporter PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${EDITOR_MODEL_IMPORTER_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${EDITOR_MODEL_IMPORTER_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_MODEL_IMPORTER_LIB}"
)
set_property(TARGET PluginEditorModelImporter PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginEditorModelImporter PRIVATE cxx_std_20)
set_target_properties(PluginEditorModelImporter PROPERTIES FOLDER "Plugins/Editor/Importers")

target_compile_definitions(PluginEditorModelImporter PUBLIC EDITOR_MODEL_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorModelImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS} ${PLUGIN_DIR})

target_precompile_headers(PluginEditorModelImporter PUBLIC ${INC}/pch.hpp)

target_link_libraries(PluginEditorModelImporter ${CORE_LIBS} assimp::assimp imgui::imgui Common EditorCommon)

message(STATUS
"PluginEditorModelImporter provides CMake targets:

- EDITOR_MODEL_IMPORTER_BASE => ${EDITOR_MODEL_IMPORTER_BASE}
- EDITOR_MODEL_IMPORTER_INCLUDES => ${EDITOR_MODEL_IMPORTER_INCLUDES}
- EDITOR_MODEL_IMPORTER_BIN => ${EDITOR_MODEL_IMPORTER_BIN}
- EDITOR_MODEL_IMPORTER_LIB => ${EDITOR_MODEL_IMPORTER_LIB}

")
