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
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${COMMON_DIR}/Editor/Importer.cpp
	${ENGINECORE_DIR}/Utils/Utilities.cpp
	EntryPoint.cpp
)

set(EDITORPIPELINESETIMPORTER_HEADERS
	PipelineSetImporter.hpp
	${COMMON_DIR}/ResourcePipeline/AssetType.hpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.hpp
	${COMMON_DIR}/ResourcePipeline/Uuid.hpp
	${COMMON_DIR}/Editor/Importer.hpp
	${ENGINECORE_DIR}/Utils/Utilities.hpp
	pch.hpp
)

find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_c CONFIG REQUIRED)
find_package(SPIRV-Tools-opt CONFIG REQUIRED)
find_package(SPIRV-Tools CONFIG REQUIRED)
find_package(directx-dxc CONFIG REQUIRED)

add_library(SPIRV-Tools ALIAS SPIRV-Tools-static)
find_package(unofficial-shaderc CONFIG REQUIRED)

find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorPipelineSetImporter MODULE
	${EDITORPIPELINESETIMPORTER_SOURCES}
	${EDITORPIPELINESETIMPORTER_HEADERS}
	${EDITORPIPELINESETIMPORTER_CONVERTER_SOURCES}
	${EDITORPIPELINESETIMPORTER_CONVERTER_HEADERS}
	${CORE_UTILS}
)

target_compile_features(PluginEditorPipelineSetImporter PRIVATE cxx_std_17)
set_target_properties(PluginEditorPipelineSetImporter PROPERTIES FOLDER "Plugins/Editor/Importers")

target_compile_definitions(PluginEditorPipelineSetImporter PUBLIC EDITOR_PIPELINESET_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorPipelineSetImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS} .)

target_precompile_headers(PluginEditorPipelineSetImporter PUBLIC pch.hpp)

target_link_libraries(PluginEditorPipelineSetImporter
	${CORE_LIBS}
	Rpcrt4.lib
	spirv-cross-c spirv-cross-core spirv-cross-cpp spirv-cross-glsl spirv-cross-reflect
	glslang::glslang glslang::glslang-default-resource-limits glslang::SPIRV glslang::SPVRemapper
	unofficial::shaderc::shaderc
	SPIRV-Tools-opt SPIRV-Tools-static
	spirv-cross-reflect
	Microsoft::DirectXShaderCompiler
)
