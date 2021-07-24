#include "MaterialManager.hpp"
#include "rapidjson/document.h"
#include "EngineCore/Assets/Shaders/ShaderManager.hpp"
#include "EngineCore/Assets/Textures/TextureManager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
#include "Common/Graphics/Texture.hpp"
using namespace Grindstone;

Material& MaterialManager::LoadMaterial(const char* path) {
	Material* material = nullptr;
	if (TryGetMaterial(path, material)) {
		return *material;
	}

	return CreateMaterialFromFile(path);
}

bool MaterialManager::TryGetMaterial(const char* path, Material*& material) {
	auto& materialInMap = materials.find(path);
	if (materialInMap != materials.end()) {
		material = &materialInMap->second;
		return true;
	}

	return false;
}

Material MaterialManager::CreateMaterialFromData(std::filesystem::path relativePath, const char* data) {
	rapidjson::Document document;
	document.Parse(data);

	if (!document.HasMember("name")) {
		throw std::runtime_error("No name found in material.");
	}
	const char* name = document["name"].GetString();

	if (!document.HasMember("shader")) {
		throw std::runtime_error("No shader found in material.");
	}

	std::string shaderPath = (relativePath / document["shader"].GetString()).string();
	ShaderManager* shaderManager = EngineCore::GetInstance().shaderManager;
	Shader* shader = &shaderManager->LoadShader(shaderPath.c_str());

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().getGraphicsCore();
	GraphicsAPI::UniformBufferBinding* uniformBufferBinding = nullptr;
	GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
	char* bufferSpace = nullptr;

	auto& uniformBuffers = shader->reflectionData.uniformBuffers;
	for (auto& uniformBuffer : uniformBuffers) {
		if (uniformBuffer.name != "MaterialUbo") {
			continue;
		}

		GraphicsAPI::UniformBufferBinding::CreateInfo ubbCi{};
		ubbCi.binding = 1;
		ubbCi.shaderLocation = "MaterialUbo";
		ubbCi.size = uniformBuffer.bufferSize;
		ubbCi.stages = (GraphicsAPI::ShaderStageBit)uniformBuffer.shaderStagesBitMask;
		uniformBufferBinding = graphicsCore->CreateUniformBufferBinding(ubbCi);

		GraphicsAPI::UniformBuffer::CreateInfo ubCi{};
		ubCi.binding = uniformBufferBinding;
		ubCi.isDynamic = true;
		ubCi.size = uniformBuffer.bufferSize;
		uniformBufferObject = graphicsCore->CreateUniformBuffer(ubCi);

		if (uniformBuffer.bufferSize == 0) {
			bufferSpace = nullptr;
		}
		else {
			bufferSpace = new char[uniformBuffer.bufferSize];

			auto& parametersJson = document["parameters"].GetObject();
			for (auto& member : uniformBuffer.members) {
				auto& params = parametersJson[member.name.c_str()].GetArray();
				std::vector<float> paramArray;
				paramArray.resize(params.Size());
				for (int i = 0; i < params.Size(); ++i) {
					paramArray[i] = params[i].GetFloat();
				}

				// char* memberPos = bufferSpace + member.offset;
				memcpy(bufferSpace, paramArray.data(), member.memberSize);
			}
		}
	}

	GraphicsAPI::TextureBinding* textureBinding = nullptr;
	auto& textures = shader->reflectionData.textures;
	bool hasSamplers = document.HasMember("samplers");
	if (textures.size() > 0 && hasSamplers) {
		auto textureManager = EngineCore::GetInstance().textureManager;
		auto& samplersJson = document["samplers"].GetObject();
		for (auto& texture : textures) {
			const char* textureName = texture.name.c_str();
			if (samplersJson.HasMember(textureName)) {
				const char* texturePath = samplersJson[textureName].GetString();
				std::filesystem::path shaderPath = relativePath / texturePath;
				TextureAsset& textureAsset = textureManager->LoadTexture(shaderPath.string().c_str());

				GraphicsAPI::SingleTextureBind stb;
				stb.texture = textureAsset.texture;
				stb.address = 2;

				GraphicsAPI::TextureBinding::CreateInfo textureBindingCreateInfo{};
				textureBindingCreateInfo.textures = &stb;
				textureBindingCreateInfo.textureCount = 1;
				textureBindingCreateInfo.layout = shader->textureBindingLayout;
				textureBinding = graphicsCore->CreateTextureBinding(textureBindingCreateInfo);
				graphicsCore->BindTexture(textureBinding);
			}
		}
	}

	return {
		name,
		shaderPath.c_str(),
		shader,
		textureBinding,
		uniformBufferBinding,
		uniformBufferObject,
		bufferSpace
	};
}

Material& MaterialManager::CreateMaterialFromFile(const char* path) {
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error(std::string(path) + " material doesn't exist.");
	}

	std::filesystem::path parentDirectory = std::filesystem::path(path).parent_path();
	std::string fileContent = Utils::LoadFileText(path);
	materials[path] = CreateMaterialFromData(parentDirectory, fileContent.c_str());

	return materials[path];
}
