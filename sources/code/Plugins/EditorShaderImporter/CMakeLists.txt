set(EDITORSHADERIMPORTER_SOURCES
	ShaderImporter.cpp
	${COMMON_DIR}/ResourcePipeline/AssetType.cpp
	${COMMON_DIR}/ResourcePipeline/MetaFile.cpp
	${COMMON_DIR}/ResourcePipeline/Uuid.cpp
	${COMMON_DIR}/Editor/Importer.cpp
	${ENGINECORE_DIR}/Utils/Utilities.cpp
	EntryPoint.cpp
)

set(EDITORSHADERIMPORTER_HEADERS
	ShaderImporter.hpp
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
add_library(SPIRV-Tools ALIAS SPIRV-Tools-static)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_path(FMT_INCLUDE_DIRS "fmt/format.h")

add_library(
	PluginEditorShaderImporter MODULE
	${EDITORSHADERIMPORTER_SOURCES}
	${EDITORSHADERIMPORTER_HEADERS}
	${CORE_UTILS}
)

target_compile_features(PluginEditorShaderImporter PRIVATE cxx_std_17)

target_compile_definitions(PluginEditorShaderImporter PUBLIC EDITOR_SHADER_IMPORTER_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginEditorShaderImporter PUBLIC ${CODE_DIR} ${FMT_INCLUDE_DIRS})

target_precompile_headers(PluginEditorShaderImporter PUBLIC pch.hpp)

target_link_libraries(PluginEditorShaderImporter
	${CORE_LIBS}
	Rpcrt4.lib
	spirv-cross-c spirv-cross-core spirv-cross-cpp spirv-cross-glsl spirv-cross-reflect
	glslang::glslang glslang::glslang-default-resource-limits glslang::SPIRV glslang::SPVRemapper
	unofficial::shaderc::shaderc
	SPIRV-Tools-opt SPIRV-Tools-static
	spirv-cross-reflect
)
