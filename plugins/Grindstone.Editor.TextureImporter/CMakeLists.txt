set(EDITOR_TEXTURE_IMPORTER_BASE ${PLUGIN_DIR}/Grindstone.Editor.TextureImporter)
set(EDITOR_TEXTURE_IMPORTER_INCLUDES ${EDITOR_TEXTURE_IMPORTER_BASE}/include)
set(EDITOR_TEXTURE_IMPORTER_BIN ${EDITOR_TEXTURE_IMPORTER_BASE}/bin)
set(EDITOR_TEXTURE_IMPORTER_LIB ${EDITOR_TEXTURE_IMPORTER_BASE}/lib)
set(SRC ${EDITOR_TEXTURE_IMPORTER_BASE}/source)
set(INC ${EDITOR_TEXTURE_IMPORTER_BASE}/include)

set(EDITORTEXTUREIMPORTER_SOURCES
	${SRC}/TextureImporter.cpp



	${EDITOR_DIR}/EditorManagerInstance.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	${SRC}/EntryPoint.cpp
)

set(EDITORTEXTUREIMPORTER_HEADERS
	${INC}/TextureImporter.hpp



	${ENGINE_CORE_DIR}/Utils/Utilities.hpp
	${INC}/pch.hpp
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

set_target_properties(PluginEditorTextureImporter PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${EDITOR_TEXTURE_IMPORTER_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${EDITOR_TEXTURE_IMPORTER_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_TEXTURE_IMPORTER_LIB}"
)
target_compile_definitions(PluginEditorTextureImporter PUBLIC EDITOR_TEXTURE_IMPORTER_DLL __STDC_LIB_EXT1__ GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorTextureImporter PUBLIC ${CODE_DIR} ${STB_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS} ${PLUGIN_DIR})

target_precompile_headers(PluginEditorTextureImporter PUBLIC ${INC}/pch.hpp)

target_link_libraries(PluginEditorTextureImporter ${CORE_LIBS} Common EditorCommon)

message(STATUS
"PluginEditorTextureImporter provides CMake targets:

- EDITOR_TEXTURE_IMPORTER_BASE => ${EDITOR_TEXTURE_IMPORTER_BASE}
- EDITOR_TEXTURE_IMPORTER_INCLUDES => ${EDITOR_TEXTURE_IMPORTER_INCLUDES}
- EDITOR_TEXTURE_IMPORTER_BIN => ${EDITOR_TEXTURE_IMPORTER_BIN}
- EDITOR_TEXTURE_IMPORTER_LIB => ${EDITOR_TEXTURE_IMPORTER_LIB}

")
