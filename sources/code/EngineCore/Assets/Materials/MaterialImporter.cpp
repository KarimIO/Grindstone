#include "MaterialImporter.hpp"
#include "rapidjson/document.h"
#include "EngineCore/Assets/Shaders/ShaderImporter.hpp"
#include "EngineCore/Assets/Textures/TextureImporter.hpp"
#include "EngineCore/AssetRenderer/BaseAssetRenderer.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "Common/Graphics/Core.hpp"
#include "Common/Graphics/Texture.hpp"
using namespace Grindstone;

MaterialImporter::MaterialImporter() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	uint32_t whiteColorData = UINT32_MAX;
	GraphicsAPI::Texture::CreateInfo whiteTextureCreateInfo{};
	whiteTextureCreateInfo.debugName = "White Missing Texture";
	whiteTextureCreateInfo.data = reinterpret_cast<const char*>(&whiteColorData);
	whiteTextureCreateInfo.size = sizeof(whiteColorData);
	whiteTextureCreateInfo.width = 1;
	whiteTextureCreateInfo.height = 1;
	whiteTextureCreateInfo.format = ColorFormat::R8G8B8A8;
	whiteTextureCreateInfo.mipmaps = 1;
	whiteTextureCreateInfo.options.shouldGenerateMipmaps = false;
	whiteTexture = graphicsCore->CreateTexture(whiteTextureCreateInfo);
}

void* MaterialImporter::ProcessLoadedFile(Uuid uuid) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;

	std::string contentData;
	if (!assetManager->LoadFileText(uuid, contentData)) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "Could not find material by file.");
		return nullptr;
	}

	rapidjson::Document document;
	document.Parse(contentData.data());

	if (!document.HasMember("name")) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "No name found in material.");
		return nullptr;
	}
	const char* name = document["name"].GetString();

	if (!document.HasMember("shader")) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "No shader found in material.");
		return nullptr;
	}

	Uuid shaderUuid(document["shader"].GetString());
	ShaderAsset* shaderAsset = assetManager->GetAsset<ShaderAsset>(shaderUuid);

	if (shaderAsset == nullptr) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "Failed to load shader.");
		return nullptr;
	}

	shaderAsset->materials.emplace_back(uuid);
	auto& material = materials.emplace(uuid, MaterialAsset(uuid, name, shaderUuid));
	MaterialAsset* materialAsset = &material.first->second;

	GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
	char* bufferSpace = nullptr;

	std::vector<DescriptorSet::Binding> bindings;

	ShaderReflectionData::StructData* materialUniformBuffer = nullptr;
	auto& uniformBuffers = shaderAsset->reflectionData.uniformBuffers;
	for (auto& uniformBuffer : uniformBuffers) {
		if (uniformBuffer.name != "MaterialUbo") {
			continue;
		}

		materialUniformBuffer = &uniformBuffer;
	}

	if (materialUniformBuffer != nullptr) {
		std::string uniformBufferName = (materialAsset->name + " MaterialUbo");
		GraphicsAPI::UniformBuffer::CreateInfo ubCi{};
		ubCi.debugName = uniformBufferName.c_str();
		ubCi.isDynamic = true;
		ubCi.size = static_cast<uint32_t>(materialUniformBuffer->bufferSize);
		uniformBufferObject = graphicsCore->CreateUniformBuffer(ubCi);

		DescriptorSet::Binding uniformBufferBinding{};
		uniformBufferBinding.bindingIndex = materialUniformBuffer->bindingId;
		uniformBufferBinding.bindingType = BindingType::UniformBuffer;
		uniformBufferBinding.itemPtr = uniformBufferObject;
		uniformBufferBinding.count = 1;
		bindings.push_back(uniformBufferBinding);

		if (ubCi.size == 0) {
			bufferSpace = nullptr;
		}
		else {
			bufferSpace = new char[ubCi.size];

			auto& parametersJson = document["parameters"];
			for (auto& member : materialUniformBuffer->members) {
				rapidjson::Value& params = parametersJson[member.name.c_str()];
				std::vector<float> paramArray;
				paramArray.resize(params.Size());
				for (rapidjson::SizeType i = 0; i < params.Size(); ++i) {
					paramArray[i] = params[i].GetFloat();
				}

				// char* memberPos = bufferSpace + member.offset;
				memcpy(bufferSpace, paramArray.data(), member.memberSize);
			}

			uniformBufferObject->UpdateBuffer(bufferSpace);
		}
	}


	auto& textureReferencesFromMaterial = shaderAsset->reflectionData.textures;
	bool hasSamplers = document.HasMember("samplers");
	if (textureReferencesFromMaterial.size() > 0 && hasSamplers) {
		auto& samplersJson = document["samplers"];
		std::vector<GraphicsAPI::Texture*> textures;
		textures.resize(textureReferencesFromMaterial.size());
		for (size_t i = 0; i < textureReferencesFromMaterial.size(); ++i) {
			const char* textureName = textureReferencesFromMaterial[i].name.c_str();
			if (samplersJson.HasMember(textureName)) {
				const char* textureUuidAsString = samplersJson[textureName].GetString();
				Uuid textureUuid(textureUuidAsString);

				TextureAsset* textureAsset = assetManager->GetAsset<TextureAsset>(textureUuid);
				DescriptorSet::Binding textureBinding{};
				textureBinding.bindingIndex = textureReferencesFromMaterial[i].bindingId;
				textureBinding.bindingType = BindingType::Texture;
				textureBinding.itemPtr = textureAsset != nullptr
					? textureAsset->texture
					: whiteTexture;
				textureBinding.count = 1;
				bindings.push_back(textureBinding);
			}
			else {
				DescriptorSet::Binding textureBinding{};
				textureBinding.bindingIndex = textureReferencesFromMaterial[i].bindingId;
				textureBinding.bindingType = BindingType::Texture;
				textureBinding.itemPtr = whiteTexture;
				textureBinding.count = 1;
				bindings.push_back(textureBinding);
			}
		}
	}

	std::string descriptorSetName = (materialAsset->name + " Material Descriptor Set");
	GraphicsAPI::DescriptorSet::CreateInfo materialDescriptorSetCreateInfo{};
	materialDescriptorSetCreateInfo.debugName = descriptorSetName.c_str();
	materialDescriptorSetCreateInfo.bindings = bindings.data();
	materialDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	materialDescriptorSetCreateInfo.layout = shaderAsset->descriptorSetLayout;
	DescriptorSet* materialDescriptorSet = graphicsCore->CreateDescriptorSet(materialDescriptorSetCreateInfo);

	materialAsset->descriptorSet = materialDescriptorSet;
	materialAsset->uniformBufferObject = uniformBufferObject;
	materialAsset->buffer = bufferSpace;

	return materialAsset;
}

bool MaterialImporter::TryGetIfLoaded(Uuid uuid, void*& output) {
	auto materialInMap = materials.find(uuid);
	if (materialInMap != materials.end()) {
		output = &materialInMap->second;
		return true;
	}

	return false;
}
