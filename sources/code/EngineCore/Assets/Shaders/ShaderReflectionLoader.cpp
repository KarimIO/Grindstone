#include <filesystem>
#include "rapidjson/document.h"
#include "Common/Graphics/Formats.hpp"
#include "ShaderImporter.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "ShaderReflectionLoader.hpp"
using namespace Grindstone;

GraphicsAPI::ShaderStage GetStageFromString(std::string str) {
	if (str == "vertex") {
		return GraphicsAPI::ShaderStage::Vertex;
	}
	else if (str == "fragment") {
		return GraphicsAPI::ShaderStage::Fragment;
	}
	else if (str == "tesselationEvaluation") {
		return GraphicsAPI::ShaderStage::TesselationEvaluation;
	}
	else if (str == "tesselationControl") {
		return GraphicsAPI::ShaderStage::TesselationControl;
	}
	else if (str == "geometry") {
		return GraphicsAPI::ShaderStage::Geometry;
	}
	else if (str == "compute") {
		return GraphicsAPI::ShaderStage::Compute;
	}

	assert(false && "Incorrect");
	return GraphicsAPI::ShaderStage::All;
}

uint8_t GetShaderBitMaskFromArray(rapidjson::GenericArray<false, rapidjson::Value> array) {
	uint8_t bitfield = 0;
	for (rapidjson::SizeType i = 0; i < array.Size(); ++i) {
		auto shaderStageString = array[i].GetString();
		auto shaderStage = GetStageFromString(shaderStageString);
		bitfield |= (1 << (uint8_t)shaderStage);
	}

	return bitfield;
}

ShaderReflectionLoader::ShaderReflectionLoader(
	const char* content,
	ShaderReflectionData& data
) : outData(data) {
	document.Parse(content);
	Process();
}

void ShaderReflectionLoader::Process() {
	ProcessMetadata();
	ProcessUniformBuffers();
	ProcessTextures();
}

void ShaderReflectionLoader::ProcessMetadata() {
	if (!document.HasMember("name")) {
		throw std::runtime_error("No name found in shader reflection.");
	}
	outData.name = document["name"].GetString();

	if (!document.HasMember("renderQueue")) {
		throw std::runtime_error("No renderQueue found in shader reflection.");
	}
	outData.renderQueue = document["renderQueue"].GetString();

	if (document.HasMember("geometryRenderer")) {
		outData.geometryRenderer = document["geometryRenderer"].GetString();
	}

	if (document.HasMember("transparencyMode")) {
		outData.transparencyMode = document["transparencyMode"].GetString();
	}

	if (document.HasMember("cullMode")) {
		outData.cullMode = document["cullMode"].GetString();
	}

	if (!document.HasMember("shaderModules")) {
		throw std::runtime_error("No shaderModules found in shader reflection.");
	}
	auto shader = document["shaderModules"].GetArray();
	outData.numShaderStages = shader.Size();
	outData.shaderStagesBitMask = GetShaderBitMaskFromArray(shader);
}

void ShaderReflectionLoader::ProcessUniformBuffers() {
	if (!document.HasMember("uniformBuffers")) {
		return;
	}

	auto& uniformBuffers = document["uniformBuffers"];
	outData.uniformBuffers.reserve(uniformBuffers.Size());
	for (
		rapidjson::Value* itr = uniformBuffers.Begin();
		itr != uniformBuffers.End();
		++itr
	) {
		auto& uniformBuffer = *itr;
		auto name = uniformBuffer["name"].GetString();
		uint32_t bindingId = uniformBuffer["binding"].GetUint();
		uint32_t descriptorSetId = uniformBuffer.HasMember("descriptorSet")
			? uniformBuffer["descriptorSet"].GetUint()
			: 0;
		uint32_t bufferSize = uniformBuffer["bufferSize"].GetUint();
		auto shaderModulesArray = document["shaderModules"].GetArray();
		uint8_t shaderModulesBits = GetShaderBitMaskFromArray(shaderModulesArray);
		outData.uniformBuffers.emplace_back(name, bindingId, descriptorSetId, bufferSize, shaderModulesBits);
		auto& memberSourceArray = uniformBuffer["members"];
		auto& memberList = outData.uniformBuffers.back().members;
		memberList.reserve(memberSourceArray.Size());
		for (rapidjson::Value& memberData : memberSourceArray.GetArray()) {
			auto name = memberData["name"].GetString();
			size_t offset = memberData["offset"].GetUint();
			size_t memberSize = memberData["memberSize"].GetUint();
			memberList.emplace_back(name, name, offset, memberSize);
		}
	}
}

void ShaderReflectionLoader::ProcessTextures() {
	if (!document.HasMember("samplers")) {
		return;
	}

	auto& resourcesArray = document["samplers"];
	outData.textures.reserve(resourcesArray.Size());
	for (rapidjson::Value& texture : resourcesArray.GetArray()) {
		auto name = texture["name"].GetString();
		uint32_t bindingId = texture["binding"].GetUint();
		uint32_t descriptorSetId = texture.HasMember("descriptorSet")
			? texture["descriptorSet"].GetUint()
			: 0;
		auto shaderModulesArray = document["shaderModules"].GetArray();
		uint8_t shaderModulesBits = GetShaderBitMaskFromArray(shaderModulesArray);
		outData.textures.emplace_back(name, bindingId, descriptorSetId, shaderModulesBits);
	}
}

void Grindstone::LoadShaderReflection(const char* path, ShaderReflectionData& reflectionData) {
	ShaderReflectionLoader loader(path, reflectionData);
}
