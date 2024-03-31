#include <iostream>
#include <fstream>
#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

#include <Common/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <Editor/EditorManager.hpp>

#include "ShaderImporter.hpp"

using namespace Grindstone::Editor::Importers;

std::string GetDataTypeName(spirv_cross::SPIRType::BaseType type) {
	switch (type) {
		case spirv_cross::SPIRType::BaseType::Void: return "Void";
		case spirv_cross::SPIRType::BaseType::Boolean: return "Boolean";
		case spirv_cross::SPIRType::BaseType::SByte: return "SByte";
		case spirv_cross::SPIRType::BaseType::UByte: return "UByte";
		case spirv_cross::SPIRType::BaseType::Short: return "Short";
		case spirv_cross::SPIRType::BaseType::UShort: return "UShort";
		case spirv_cross::SPIRType::BaseType::Int: return "Int";
		case spirv_cross::SPIRType::BaseType::UInt: return "UInt";
		case spirv_cross::SPIRType::BaseType::Int64: return "Int64";
		case spirv_cross::SPIRType::BaseType::UInt64: return "UInt64";
		case spirv_cross::SPIRType::BaseType::AtomicCounter: return "AtomicCounter";
		case spirv_cross::SPIRType::BaseType::Half: return "Half";
		case spirv_cross::SPIRType::BaseType::Float: return "Float";
		case spirv_cross::SPIRType::BaseType::Double: return "Double";
		case spirv_cross::SPIRType::BaseType::Struct: return "Struct";
		case spirv_cross::SPIRType::BaseType::Image: return "Image";
		case spirv_cross::SPIRType::BaseType::SampledImage: return "SampledImage";
		case spirv_cross::SPIRType::BaseType::Sampler: return "Sampler";
		case spirv_cross::SPIRType::BaseType::AccelerationStructure: return "AccelerationStructure";
		case spirv_cross::SPIRType::BaseType::RayQuery: return "RayQuery";
		default: return "Unknown";
	}
}

std::string ReadTextFile(const char* filename) {
	std::ifstream ifs(filename);

	if (!ifs.is_open()) {
		throw std::runtime_error(std::string("Failed to open file ") + filename);
	}

	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	return content;
}

ShaderImporter::Texture* FindTextureAlreadyReflected(
	std::vector<ShaderImporter::Texture>& textures,
	const std::string& resourceName
) {
	for (size_t i = 0; i < textures.size(); ++i) {
		if (textures[i].name == resourceName) {
			return &textures[i];
		}
	}

	return nullptr;
}

void ReflectImages(
	ShaderImporter::ShaderType shaderType,
	std::vector<ShaderImporter::Texture>& textures,
	spirv_cross::Compiler& compiler,
	spirv_cross::SmallVector<spirv_cross::Resource>& resourceList
) {
	for (const auto& resource : resourceList) {
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		auto& resourceName = resource.name;

		auto textureReflected = FindTextureAlreadyReflected(textures, resourceName);
		if (textureReflected != nullptr) {
			textureReflected->shaderPasses.push_back(shaderType);
			continue;
		}

		textures.emplace_back(resourceName, binding, descriptorSet);
		textures.back().shaderPasses.push_back(shaderType);
	}
}

ShaderImporter::UniformBuffer* FindUniformAlreadyReflected(
	std::vector<ShaderImporter::UniformBuffer>& uniformBuffers,
	const std::string& resourceName
) {
	for (size_t i = 0; i < uniformBuffers.size(); ++i) {
		if (uniformBuffers[i].name == resourceName) {
			return &uniformBuffers[i];
		}
	}

	return nullptr;
}

void ReflectStruct(
	ShaderImporter::ShaderType shaderType,
	std::vector<ShaderImporter::UniformBuffer>& uniformBuffers,
	spirv_cross::Compiler& compiler,
	spirv_cross::SmallVector<spirv_cross::Resource>& resourceList
) {
	for (const auto& resource : resourceList) {
		spirv_cross::SPIRType bufferType = compiler.get_type(resource.base_type_id);
		size_t bufferSize = compiler.get_declared_struct_size(bufferType);
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		uint32_t memberCount = static_cast<uint32_t>(bufferType.member_types.size());
		auto& resourceName = resource.name;

		auto ubReflected = FindUniformAlreadyReflected(uniformBuffers, resourceName);
		if (ubReflected != nullptr) {
			ubReflected->shaderPasses.push_back(shaderType);
			continue;
		}

		uniformBuffers.emplace_back(resourceName.c_str(), binding, descriptorSet, bufferSize);
		auto& uniformBuffer = uniformBuffers.back();
		uniformBuffer.shaderPasses.push_back(shaderType);

		for (uint32_t i = 0; i < memberCount; i++) {
			auto& memberType = compiler.get_type(bufferType.member_types[i]);
			size_t memberSize = compiler.get_declared_struct_member_size(bufferType, i);
			uint32_t offset = compiler.type_struct_member_offset(bufferType, i);
			const std::string& name = compiler.get_member_name(bufferType.self, i);
			auto& typeStr = GetDataTypeName(memberType.basetype);
			memberType.vecsize;
			memberType.columns;

			uniformBuffer.members.emplace_back(name, typeStr, offset, memberSize);
		}
	}
}

shaderc_shader_kind GetShaderTypeForShaderc(ShaderImporter::ShaderType type) {
	switch(type) {
	case ShaderImporter::ShaderType::Vertex: return shaderc_glsl_vertex_shader;
	case ShaderImporter::ShaderType::Fragment: return shaderc_glsl_fragment_shader;
	case ShaderImporter::ShaderType::Geometry: return shaderc_glsl_geometry_shader;
	case ShaderImporter::ShaderType::Compute: return shaderc_glsl_compute_shader;
	case ShaderImporter::ShaderType::TesselationControl: return shaderc_glsl_tess_control_shader;
	case ShaderImporter::ShaderType::TesselationEvaluation: return shaderc_glsl_tess_evaluation_shader;
	default: throw std::runtime_error("Invalid Shader Type!");
	}
}

void ShaderImporter::Import(const std::filesystem::path& inputPath) {
	this->inputPath = inputPath;

	metaFile = new MetaFile();
	metaFile->Load(inputPath);
	sourceFileContents = ReadTextFile(inputPath.string().c_str());

	Process();
	metaFile->Save();

	Uuid uuid;
	if (metaFile->TryGetDefaultSubassetUuid(uuid)) {
		Editor::Manager::GetEngineCore().assetManager->QueueReloadAsset(AssetType::Shader, uuid);
	}
}

void ShaderImporter::Process() {
	shaderName = ExtractField("#name");
	Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(shaderName, AssetType::Shader);
	baseOutputPath = (Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString()).string();

	renderQueue = ExtractField("#renderQueue");
	geometryRenderer = ExtractField("#geometryRenderer");
	transparencyMode = ExtractField("#transparencyMode");
	cullMode = ExtractField("#cullMode");
	ExtractSubmodules();
	WriteReflectionDocument();
}

void ShaderImporter::WriteReflectionStruct(std::vector<UniformBuffer>& structs) {
	reflectionWriter.StartArray();
	for (auto& structMeta : structs) {
		reflectionWriter.StartObject();
		reflectionWriter.Key("name");
		reflectionWriter.String(structMeta.name.c_str());
		reflectionWriter.Key("binding");
		reflectionWriter.Uint(static_cast<unsigned int>(structMeta.binding));
		reflectionWriter.Key("descriptorSet");
		reflectionWriter.Uint(static_cast<unsigned int>(structMeta.descriptorSet));
		reflectionWriter.Key("bufferSize");
		reflectionWriter.Uint(static_cast<unsigned int>(structMeta.bufferSize));

		reflectionWriter.Key("usedIn");
		reflectionWriter.StartArray();
		for (auto & shaderPass : structMeta.shaderPasses) {
			reflectionWriter.String(GetShaderTypeAsString(shaderPass));
		}
		reflectionWriter.EndArray();

		reflectionWriter.Key("members");
		reflectionWriter.StartArray();
		for (auto& member : structMeta.members) {
			reflectionWriter.StartObject();
			reflectionWriter.Key("name");
			reflectionWriter.String(member.name.c_str());
			reflectionWriter.Key("offset");
			reflectionWriter.Uint(static_cast<unsigned int>(member.offset));
			reflectionWriter.Key("memberSize");
			reflectionWriter.Uint(static_cast<unsigned int>(member.memberSize));
			reflectionWriter.Key("type");
			reflectionWriter.String(member.type.c_str());
			reflectionWriter.EndObject();
		}
		reflectionWriter.EndArray();

		reflectionWriter.EndObject();
	}
	reflectionWriter.EndArray();
}

void ShaderImporter::WriteReflectionImage(std::vector<Texture>& resources) {
	reflectionWriter.StartArray();
	for (auto & resource : resources) {
		reflectionWriter.StartObject();
		reflectionWriter.Key("name");
		reflectionWriter.String(resource.name.c_str());
		reflectionWriter.Key("binding");
		reflectionWriter.Uint(static_cast<unsigned int>(resource.binding));
		reflectionWriter.Key("descriptorSet");
		reflectionWriter.Uint(static_cast<unsigned int>(resource.descriptorSet));

		reflectionWriter.Key("usedIn");
		reflectionWriter.StartArray();
		for (auto& shaderPass : resource.shaderPasses) {
			reflectionWriter.String(GetShaderTypeAsString(shaderPass));
		}
		reflectionWriter.EndArray();

		reflectionWriter.EndObject();
	}
	reflectionWriter.EndArray();
}

void ShaderImporter::WriteReflectionDocument() {
	reflectionWriter.StartObject();

	reflectionWriter.Key("name");
	reflectionWriter.String(shaderName.c_str());

	reflectionWriter.Key("geometryRenderer");
	reflectionWriter.String(geometryRenderer.c_str());

	reflectionWriter.Key("renderQueue");
	reflectionWriter.String(renderQueue.c_str());

	reflectionWriter.Key("transparencyMode");
	reflectionWriter.String(transparencyMode.c_str());

	reflectionWriter.Key("cullMode");
	reflectionWriter.String(cullMode.c_str());

	reflectionWriter.Key("shaderModules");
	reflectionWriter.StartArray();
	for (auto & shaderPass : shaderPasses) {
		reflectionWriter.String(GetShaderTypeAsString(shaderPass));
	}
	reflectionWriter.EndArray();

	reflectionWriter.Key("uniformBuffers");
	WriteReflectionStruct(uniformBuffers);

	reflectionWriter.Key("textures");
	WriteReflectionImage(textures);

	reflectionWriter.Key("samplers");
	WriteReflectionImage(samplers);

	reflectionWriter.EndObject();

	OutputStringToFile("", reflectionStringBuffer.GetString());
}

std::string ShaderImporter::ExtractField(const char* fieldKey) {
	auto fieldPos = sourceFileContents.find(fieldKey);

	if (fieldPos == std::string::npos) {
		return "";
	}

	auto newLinePos = sourceFileContents.find('\n', fieldPos);
	auto valuePos = fieldPos + strlen(fieldKey) + 1;
	return sourceFileContents.substr(valuePos, newLinePos - valuePos);
}
		
void ShaderImporter::ExtractSubmodules() {
	const char* fieldKey = "#shaderModule";
	const size_t fieldKeyLength = strlen(fieldKey) + 1;

	size_t beginSearchPos = 0;

	while (true) {
		size_t fieldPos = sourceFileContents.find(fieldKey, beginSearchPos);
		if (fieldPos == -1) {
			return;
		}

		size_t newLinePos = sourceFileContents.find('\n', fieldPos);
		size_t valuePos = fieldPos + fieldKeyLength;

		size_t beginPos = newLinePos + 1;
		size_t endPos = sourceFileContents.find("#endShaderModule", newLinePos);

		std::string shaderTypeString = sourceFileContents.substr(valuePos, newLinePos - valuePos);
		auto shaderType = GetShaderTypeFromString(shaderTypeString);
		std::string glsl = sourceFileContents.substr(beginPos, endPos - beginPos);
		std::string extension = GetShaderTypeExtension(shaderType);
		ProcessSubmodule(shaderType, extension.c_str(), glsl.c_str());

		beginSearchPos = endPos;
	}
}

ShaderImporter::ShaderType ShaderImporter::GetShaderTypeFromString(std::string& str) {
	if (str == "vertex") {
		return ShaderType::Vertex;
	}
	else if (str == "fragment") {
		return ShaderType::Fragment;
	}
	else if (str == "geometry") {
		return ShaderType::Geometry;
	}
	else if (str == "compute") {
		return ShaderType::Compute;
	}
	else if (str == "tesselationControl") {
		return ShaderType::TesselationControl;
	}
	else if (str == "tesselationEvaluation") {
		return ShaderType::TesselationEvaluation;
	}

	throw std::runtime_error("Invalid Shader Type!");
}

const char* ShaderImporter::GetShaderTypeExtension(ShaderType type) {
	switch(type) {
	case ShaderType::Vertex: return ".vert";
	case ShaderType::Fragment: return ".frag";
	case ShaderType::Geometry: return ".geom";
	case ShaderType::Compute: return ".comp";
	case ShaderType::TesselationControl: return ".ctrl";
	case ShaderType::TesselationEvaluation: return ".eval";
	default: throw std::runtime_error("Invalid Shader Type!");
	}
}
		
const char* ShaderImporter::GetShaderTypeAsString(ShaderType type) {
	switch(type) {
	case ShaderType::Vertex: return "vertex";
	case ShaderType::Fragment: return "fragment";
	case ShaderType::Geometry: return "geometry";
	case ShaderType::Compute: return "compute";
	case ShaderType::TesselationControl: return "tesselationControl";
	case ShaderType::TesselationEvaluation: return "tesselationEvaluation";
	default: throw std::runtime_error("Invalid Shader Type!");
	}
}

void ShaderImporter::ProcessSubmodule(ShaderType shaderType, const char* extension, const char* glslSource) {
	std::vector<uint32_t> vkSpirv = ConvertToSpirv(shaderType, extension, glslSource);

	if (vkSpirv.size() == 0) {
		return;
	}

	/* TODO: Re-implement this by using preprocessing to handle the descriptor set issue
	{
		auto opengGlsl = ConvertToOpenglGlsl(extension, vkSpirv);
		ConvertToOpenglSpirv(shaderType, extension, glslSource);
	}
	*/
	ReflectResources(shaderType, vkSpirv);
}

std::vector<uint32_t> ShaderImporter::ConvertToSpirv(ShaderType shaderType, const char* extension, const char* shaderModuleGlsl) {
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	auto result = compiler.CompileGlslToSpv(
		shaderModuleGlsl,
		GetShaderTypeForShaderc(shaderType),
		inputPath.string().c_str(),
		options
	);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		Editor::Manager::Print(LogSeverity::Error, "{}", result.GetErrorMessage().c_str());
		return std::vector<uint32_t>();
	}

	auto vkSpirv = std::vector<uint32_t>(result.cbegin(), result.cend());

	OutputUint32ToFile((std::string(extension) + ".vk.spv").c_str(), vkSpirv);

	return vkSpirv;
}

std::string ShaderImporter::ConvertToOpenglGlsl(const char* extension, std::vector<uint32_t>& vkSpirv) {
	spirv_cross::CompilerGLSL glslTranspiler(vkSpirv);

	spirv_cross::CompilerGLSL::Options glslTranspilerOptions;
	glslTranspilerOptions.version = 450;
	glslTranspiler.set_common_options(glslTranspilerOptions);

	std::string openglGlsl = glslTranspiler.compile();

	OutputStringToFile((std::string(extension) + ".ogl.glsl").c_str(), openglGlsl.c_str());

	return openglGlsl;
}

void ShaderImporter::ConvertToOpenglSpirv(ShaderType shaderType, const char* extension, const char* opengGlsl) {
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
	options.SetOptimizationLevel(shaderc_optimization_level_performance);

	auto result = compiler.CompileGlslToSpv(
		opengGlsl,
		GetShaderTypeForShaderc(shaderType),
		inputPath.string().c_str(),
		options
	);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		Editor::Manager::Print(LogSeverity::Error, result.GetErrorMessage().c_str());
		return;
	}

	OutputUint32ToFile((std::string(extension) + ".ogl.spv").c_str(), std::vector<uint32_t>(result.cbegin(), result.cend()));
}

void ShaderImporter::ReflectResources(ShaderType shaderType, std::vector<uint32_t>& vkSpirv) {
	spirv_cross::Compiler compiler(vkSpirv);
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	shaderPasses.push_back(shaderType);

	std::string resourcesStr;
	if (resources.uniform_buffers.size()) {
		ReflectStruct(shaderType, uniformBuffers, compiler, resources.uniform_buffers);
	}
	if (resources.push_constant_buffers.size()) {
		// reflectStruct(shaderType, pushConstants, compiler, resources.push_constant_buffers);
	}
	if (resources.sampled_images.size()) {
		ReflectImages(shaderType, samplers, compiler, resources.sampled_images);
	}
	if (resources.separate_images.size()) {
		ReflectImages(shaderType, textures, compiler, resources.separate_images);
	}
}

void ShaderImporter::OutputStringToFile(const char* extension, const char* content) {
	std::string outputFilename = baseOutputPath + extension;
	std::ofstream file(outputFilename);
	file.write((const char*)content, strlen(content));
	file.flush();
	file.close();
}

void ShaderImporter::OutputUint32ToFile(const char* extension, std::vector<uint32_t>& content) {
	std::string outputFilename = baseOutputPath + extension;
	std::ofstream file(outputFilename, std::ios::out | std::ios::binary);
	auto fileSize = sizeof(uint32_t) * content.size();
	file.write((const char*)content.data(), fileSize);
	file.flush();
	file.close();
}

void Grindstone::Editor::Importers::ImportShadersFromGlsl(const std::filesystem::path& filePath) {
	Importers::ShaderImporter importer;
	importer.Import(filePath);
}
