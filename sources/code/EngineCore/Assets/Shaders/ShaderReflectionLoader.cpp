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
	std::string content = Utils::LoadFileText(path.c_str());
	rapidjson::Document document;
	document.Parse(content.c_str());

	Process();
}

void ShaderReflectionLoader::Process() {
	ProcessMetadata();
	ProcessUniformBuffers();
}

void ShaderReflectionLoader::ProcessMetadata() {
	outData.name = document["name"].GetString();

	auto& shader = document["shaderModules"].GetArray();
	outData.numShaderStages = shader.Size();
	outData.shaderStagesBitMask = GetShaderBitMaskFromArray(shader);
}

void ShaderReflectionLoader::ProcessUniformBuffers() {
	if (!document.HasMember("uniformBuffers")) {
		return;
	}

	auto& uniformBuffers = document["uniformBuffers"];
	outData.uniformBuffers.resize(uniformBuffers.Size());
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
	if (!document.HasMember("uniformBuffers")) {
		return;
	}

	auto& resources = document["uniformBuffers"];
	outData.uniformBuffers.resize(resources.Size());
	for (
		rapidjson::Value* itr = resources.Begin();
		itr != resources.End();
		++itr
	) {
		auto& uniformBuffer = itr->GetObject();
		auto name = uniformBuffer["name"].GetString();
		size_t bindingId = uniformBuffer["binding"].GetUint();
	}
}

void Grindstone::LoadShaderReflection(const char* path, ShaderReflectionData& reflectionData) {
	ShaderReflectionLoader loader(path, reflectionData);
}
