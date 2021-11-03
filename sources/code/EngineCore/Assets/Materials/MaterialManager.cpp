#include "MaterialManager.hpp"
#include "rapidjson/document.h"
#include "EngineCore/Assets/Shaders/ShaderManager.hpp"
#include "EngineCore/Assets/Textures/TextureManager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
#include "Common/Graphics/Texture.hpp"
using namespace Grindstone;

Material& MaterialManager::LoadMaterial(BaseAssetRenderer* assetRenderer, const char* path) {
	Material* material = nullptr;
	if (!TryGetMaterial(path, material)) {
		material = &CreateMaterialFromFile(assetRenderer, path);
	}

	return *material;
}

void MaterialManager::ReloadMaterialIfLoaded(const char* path) {}

bool MaterialManager::TryGetMaterial(const char* path, Material*& material) {
	auto& materialInMap = materials.find(path);
	if (materialInMap != materials.end()) {
		material = &materialInMap->second;
		return true;
	}

	return false;
}

void MaterialManager::CreateMaterialFromData(
	std::filesystem::path relativePath,
	Material& material,
	BaseAssetRenderer* assetRenderer,
	const char* data
) {
	rapidjson::Document document;
	document.Parse(data);

	if (!document.HasMember("name")) {
		throw std::runtime_error("No name found in material.");
	}
	const char* name = document["name"].GetString();

	if (!document.HasMember("shader")) {
		throw std::runtime_error("No shader found in material.");
	}

	std::string shaderPath = document["shader"].GetString();
	ShaderManager* shaderManager = EngineCore::GetInstance().shaderManager;
	Shader* shader = &shaderManager->LoadShader(assetRenderer, shaderPath.c_str());
	shader->materials.push_back(&material);

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	GraphicsAPI::UniformBufferBinding* uniformBufferBinding = nullptr;
	GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
	char* bufferSpace = nullptr;

	auto& uniformBuffers = shader->reflectionData.uniformBuffers;
	for (auto& uniformBuffer : uniformBuffers) {
		if (uniformBuffer.name != "MaterialUbo") {
			continue;
		}

		GraphicsAPI::UniformBufferBinding::CreateInfo ubbCi{};
		ubbCi.binding = 2;
		ubbCi.shaderLocation = "MaterialUbo";
		ubbCi.size = (uint32_t)uniformBuffer.bufferSize;
		ubbCi.stages = (GraphicsAPI::ShaderStageBit)uniformBuffer.shaderStagesBitMask;
		uniformBufferBinding = graphicsCore->CreateUniformBufferBinding(ubbCi);

		GraphicsAPI::UniformBuffer::CreateInfo ubCi{};
		ubCi.binding = uniformBufferBinding;
		ubCi.isDynamic = true;
		ubCi.size = (uint32_t)uniformBuffer.bufferSize;
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
				for (rapidjson::SizeType i = 0; i < params.Size(); ++i) {
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
		std::vector<GraphicsAPI::SingleTextureBind> textureBinds;
		textureBinds.resize(textures.size());
		for (size_t i = 0; i < textures.size(); ++i) {
			const char* textureName = textures[i].name.c_str();
			if (samplersJson.HasMember(textureName)) {
				const char* texturePath = samplersJson[textureName].GetString();
				std::filesystem::path shaderPath = relativePath / texturePath;
				TextureAsset& textureAsset = strcmp(texturePath, "") != 0
					? textureManager->LoadTexture(shaderPath.string().c_str())
					: textureManager->GetDefaultTexture();

				GraphicsAPI::SingleTextureBind& stb = textureBinds[i];
				stb.texture = textureAsset.texture;
				stb.address = textures[i].bindingId;
			}
		}

		GraphicsAPI::TextureBinding::CreateInfo textureBindingCreateInfo{};
		textureBindingCreateInfo.textures = textureBinds.data();
		textureBindingCreateInfo.textureCount = textureBinds.size();
		textureBindingCreateInfo.layout = shader->textureBindingLayout;
		textureBinding = graphicsCore->CreateTextureBinding(textureBindingCreateInfo);
		graphicsCore->BindTexture(textureBinding);
	}

	material.name = name;
	material.shaderPath = shaderPath.c_str();
	material.shader = shader;
	material.textureBinding = textureBinding;
	material.uniformBufferBinding = uniformBufferBinding;
	material.uniformBufferObject = uniformBufferObject;
	material.buffer = bufferSpace;
}

Material& MaterialManager::CreateMaterialFromFile(BaseAssetRenderer* assetRenderer, const char* path) {
	std::filesystem::path completePath = std::filesystem::path("../compiledAssets/") / path;
	if (!std::filesystem::exists(completePath)) {
		throw std::runtime_error(completePath.string() + " material doesn't exist.");
	}

	std::filesystem::path parentDirectory = completePath.parent_path();
	std::string fileContent = Utils::LoadFileText(completePath.string().c_str());
	Material& material = materials[path];
	CreateMaterialFromData(parentDirectory, material, assetRenderer, fileContent.c_str());

	return material;
}
