#include <filesystem>
#include "rapidjson/document.h"
#include "Common/Graphics/Formats.hpp"
#include "ShaderManager.hpp"
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
	const char* basePath,
	ShaderReflectionData& data
) : outData(data) {
	std::string path = std::string(basePath) + ".reflect.json";

	if (!std::filesystem::exists(path)) {
		throw std::runtime_error(path + " not found!");
	}

	std::string content = Utils::LoadFileText(path.c_str());
	document.Parse(content.c_str());

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

	if (!document.HasMember("shaderModules")) {
		throw std::runtime_error("No shaderModules found in shader reflection.");
	}
	auto& shader = document["shaderModules"].GetArray();
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
		auto& uniformBuffer = itr->GetObject();
		auto name = uniformBuffer["name"].GetString();
		size_t bindingId = uniformBuffer["binding"].GetUint();
		size_t bufferSize = uniformBuffer["bufferSize"].GetUint();
		auto& shaderModules = document["shaderModules"].GetArray();
		uint8_t shaderModulesBits = GetShaderBitMaskFromArray(shaderModules);
		outData.uniformBuffers.emplace_back(name, bindingId, bufferSize, shaderModulesBits);
		auto& memberSource = uniformBuffer["members"];
		auto& memberList = outData.uniformBuffers.back().members;
		memberList.reserve(memberSource.Size());
		for (
			rapidjson::Value* memberItr = memberSource.Begin();
			memberItr != memberSource.End();
			++memberItr
		) {
			auto& memberData = memberItr->GetObject();
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

	auto& resources = document["samplers"];
	outData.textures.reserve(resources.Size());
	for (
		rapidjson::Value* itr = resources.Begin();
		itr != resources.End();
		++itr
	) {
		auto& texture = itr->GetObject();
		auto name = texture["name"].GetString();
		size_t bindingId = texture["binding"].GetUint();
		auto& shaderModules = document["shaderModules"].GetArray();
		uint8_t shaderModulesBits = GetShaderBitMaskFromArray(shaderModules);
		outData.textures.emplace_back(name, bindingId, shaderModulesBits);
	}
}

void Grindstone::LoadShaderReflection(const char* path, ShaderReflectionData& reflectionData) {
	ShaderReflectionLoader loader(path, reflectionData);
}
