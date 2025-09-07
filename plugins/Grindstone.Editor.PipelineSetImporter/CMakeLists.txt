set(PipelineSetConverter "PipelineSet/Converter")
set(EDITOR_PIPELINESET_IMPORTER_BASE ${PLUGIN_DIR}/Grindstone.Editor.PipelineSetImporter)
set(EDITOR_PIPELINESET_IMPORTER_INCLUDES ${EDITOR_PIPELINESET_IMPORTER_BASE}/include)
set(EDITOR_PIPELINESET_IMPORTER_BIN ${EDITOR_PIPELINESET_IMPORTER_BASE}/bin)
set(EDITOR_PIPELINESET_IMPORTER_LIB ${EDITOR_PIPELINESET_IMPORTER_BASE}/lib)
set(SRC ${EDITOR_PIPELINESET_IMPORTER_BASE}/source)
set(INC ${EDITOR_PIPELINESET_IMPORTER_BASE}/include)

set(EDITORPIPELINESETIMPORTER_CONVERTER_SOURCES
	${SRC}/${PipelineSetConverter}/Output.cpp
	${SRC}/${PipelineSetConverter}/Parser.cpp
	${SRC}/${PipelineSetConverter}/PipelineSetConverter.cpp
	${SRC}/${PipelineSetConverter}/Scanner.cpp
	${SRC}/${PipelineSetConverter}/ShaderCompiler.cpp
	${SRC}/${PipelineSetConverter}/StateResolver.cpp
	${SRC}/${PipelineSetConverter}/Token.cpp
)

source_group("Converter\\Source Files" FILES ${EDITORPIPELINESETIMPORTER_CONVERTER_SOURCES})

set(EDITORPIPELINESETIMPORTER_CONVERTER_HEADERS
	${INC}/${PipelineSetConverter}/CompilationArtifacts.hpp
	${INC}/${PipelineSetConverter}/CompilationOptions.hpp
	${INC}/${PipelineSetConverter}/Keyword.hpp
	${INC}/${PipelineSetConverter}/Output.hpp
	${INC}/${PipelineSetConverter}/Parser.hpp
	${INC}/${PipelineSetConverter}/ParseTree.hpp
	${INC}/${PipelineSetConverter}/ParameterType.hpp
	${INC}/${PipelineSetConverter}/PipelineSetConditioner.hpp
	${INC}/${PipelineSetConverter}/ResolvedStateTree.hpp
	${INC}/${PipelineSetConverter}/Scanner.hpp
	${INC}/${PipelineSetConverter}/ShaderCompiler.hpp
	${INC}/${PipelineSetConverter}/StateResolver.hpp
	${INC}/${PipelineSetConverter}/Token.hpp
	${INC}/PipelineSet/Log.hpp
	${INC}/PipelineSet/WriteCallback.hpp
)
source_group("Converter\\Header Files" FILES ${EDITORPIPELINESETIMPORTER_CONVERTER_HEADERS})

set(EDITORPIPELINESETIMPORTER_SOURCES
	${SRC}/PipelineSetImporter.cpp



	${EDITOR_DIR}/EditorManagerInstance.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	${SRC}/EntryPoint.cpp
)

set(EDITORPIPELINESETIMPORTER_HEADERS
	${INC}/PipelineSetImporter.hpp



	${ENGINE_CORE_DIR}/Utils/Utilities.hpp
	${INC}/pch.hpp
)

find_package(unofficial-spirv-reflect CONFIG REQUIRED)
find_package(directx-dxc CONFIG REQUIRED)

find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorPipelineSetImporter MODULE
	${EDITORPIPELINESETIMPORTER_SOURCES}
	${EDITORPIPELINESETIMPORTER_HEADERS}
	${EDITORPIPELINESETIMPORTER_CONVERTER_SOURCES}
	${EDITORPIPELINESETIMPORTER_CONVERTER_HEADERS}
	${CORE_UTILS}
)

set_property(TARGET PluginEditorPipelineSetImporter PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginEditorPipelineSetImporter PRIVATE cxx_std_20)
set_target_properties(PluginEditorPipelineSetImporter PROPERTIES FOLDER "Plugins/Editor/Importers")

target_compile_definitions(PluginEditorPipelineSetImporter PUBLIC EDITOR_PIPELINESET_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorPipelineSetImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS} ${PLUGIN_DIR})

set_target_properties(PluginEditorPipelineSetImporter PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${EDITOR_PIPELINESET_IMPORTER_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${EDITOR_PIPELINESET_IMPORTER_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_PIPELINESET_IMPORTER_LIB}"
)
target_precompile_headers(PluginEditorPipelineSetImporter PUBLIC ${INC}/pch.hpp)

target_link_libraries(PluginEditorPipelineSetImporter
	${CORE_LIBS}
	unofficial::spirv-reflect
	Microsoft::DirectXShaderCompiler
	Common EditorCommon
)

message(STATUS
"PluginEditorPipelineSetImporter provides CMake targets:

- EDITOR_PIPELINESET_IMPORTER_BASE => ${EDITOR_PIPELINESET_IMPORTER_BASE}
- EDITOR_PIPELINESET_IMPORTER_INCLUDES => ${EDITOR_PIPELINESET_IMPORTER_INCLUDES}
- EDITOR_PIPELINESET_IMPORTER_BIN => ${EDITOR_PIPELINESET_IMPORTER_BIN}
- EDITOR_PIPELINESET_IMPORTER_LIB => ${EDITOR_PIPELINESET_IMPORTER_LIB}

")
