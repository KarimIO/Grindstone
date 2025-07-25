set(PipelineSetConverter "PipelineSet/Converter")

set(EDITORPIPELINESETIMPORTER_CONVERTER_SOURCES
	${PipelineSetConverter}/Output.cpp
	${PipelineSetConverter}/Parser.cpp
	${PipelineSetConverter}/PipelineSetConverter.cpp
	${PipelineSetConverter}/Scanner.cpp
	${PipelineSetConverter}/ShaderCompiler.cpp
	${PipelineSetConverter}/StateResolver.cpp
	${PipelineSetConverter}/Token.cpp
)

source_group("Converter\\Source Files" FILES ${EDITORPIPELINESETIMPORTER_CONVERTER_SOURCES})

set(EDITORPIPELINESETIMPORTER_CONVERTER_HEADERS
	${PipelineSetConverter}/CompilationArtifacts.hpp
	${PipelineSetConverter}/CompilationOptions.hpp
	${PipelineSetConverter}/Keyword.hpp
	${PipelineSetConverter}/Output.hpp
	${PipelineSetConverter}/Parser.hpp
	${PipelineSetConverter}/ParseTree.hpp
	${PipelineSetConverter}/ParameterType.hpp
	${PipelineSetConverter}/PipelineSetConditioner.hpp
	${PipelineSetConverter}/ResolvedStateTree.hpp
	${PipelineSetConverter}/Scanner.hpp
	${PipelineSetConverter}/ShaderCompiler.hpp
	${PipelineSetConverter}/StateResolver.hpp
	${PipelineSetConverter}/Token.hpp
	PipelineSet/Log.hpp
	PipelineSet/WriteCallback.hpp
)
source_group("Converter\\Header Files" FILES ${EDITORPIPELINESETIMPORTER_CONVERTER_HEADERS})

set(EDITORPIPELINESETIMPORTER_SOURCES
	PipelineSetImporter.cpp
	${COMMON_DIR}/ResourcePipeline/AssetType.cpp
	${COMMON_DIR}/ResourcePipeline/ImporterSettings.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${EDITOR_DIR}/EditorManagerInstance.cpp
	${ENGINE_CORE_DIR}/Utils/Utilities.cpp
	EntryPoint.cpp
)

set(EDITORPIPELINESETIMPORTER_HEADERS
	PipelineSetImporter.hpp
	${COMMON_DIR}/ResourcePipeline/AssetType.hpp
	${COMMON_DIR}/ResourcePipeline/ImporterSettings.hpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.hpp
	${COMMON_DIR}/ResourcePipeline/Uuid.hpp
	${COMMON_DIR}/Editor/Importer.hpp
	${ENGINE_CORE_DIR}/Utils/Utilities.hpp
	pch.hpp
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
target_include_directories(PluginEditorPipelineSetImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS} ${CODE_DIR}/Plugins/EditorPipelineSetImporter)

target_precompile_headers(PluginEditorPipelineSetImporter PUBLIC pch.hpp)

target_link_libraries(PluginEditorPipelineSetImporter
	${CORE_LIBS}
	unofficial::spirv-reflect
	Microsoft::DirectXShaderCompiler
)
