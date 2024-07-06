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

	uint32_t blackColorData = 0;
	GraphicsAPI::Texture::CreateInfo blackTextureCreateInfo{};
	blackTextureCreateInfo.debugName = "Black Missing Texture";
	blackTextureCreateInfo.data = reinterpret_cast<const char*>(&blackColorData);
	blackTextureCreateInfo.size = sizeof(blackColorData);
	blackTextureCreateInfo.width = 1;
	blackTextureCreateInfo.height = 1;
	blackTextureCreateInfo.format = ColorFormat::RGBA8;
	blackTextureCreateInfo.mipmaps = 1;
	blackTextureCreateInfo.options.shouldGenerateMipmaps = false;
	missingTexture = graphicsCore->CreateTexture(blackTextureCreateInfo);
}

void* MaterialImporter::ProcessLoadedFile(Uuid uuid) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;

	std::string assetName, contentData;
	if (!assetManager->LoadFileText(AssetType::Material, uuid, assetName, contentData)) {
		std::string error = "Could not find material with id " + uuid.ToString() + ".";
		EngineCore::GetInstance().Print(LogSeverity::Error, error.c_str());
		return nullptr;
	}

	rapidjson::Document document;
	if (document.Parse(contentData.data()).HasParseError()) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "Unable to parse material.");
		return nullptr;
	}

	if (!document.HasMember("shader")) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "No shader found in material.");
		return nullptr;
	}

	Uuid shaderUuid(document["shader"].GetString());
	ShaderAsset* shaderAsset = assetManager->IncrementAssetUse<ShaderAsset>(shaderUuid);

	if (shaderAsset == nullptr) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "Failed to load shader.");
		return nullptr;
	}

	auto& reflectionData = shaderAsset->reflectionData;

	auto& material = assets.emplace(uuid, MaterialAsset(uuid, assetName, shaderUuid));
	MaterialAsset* materialAsset = &material.first->second;

	std::vector<DescriptorSet::Binding> bindings;
	SetupUniformBuffer(document, reflectionData, bindings, materialAsset->name, materialAsset);
	SetupSamplers(document, reflectionData, bindings);

	std::string descriptorSetName = (materialAsset->name + " Material Descriptor Set");

	GraphicsAPI::DescriptorSet::CreateInfo materialDescriptorSetCreateInfo{};
	materialDescriptorSetCreateInfo.debugName = descriptorSetName.c_str();
	materialDescriptorSetCreateInfo.bindings = bindings.data();
	materialDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	materialDescriptorSetCreateInfo.layout = shaderAsset->descriptorSetLayout;
	DescriptorSet* materialDescriptorSet = graphicsCore->CreateDescriptorSet(materialDescriptorSetCreateInfo);

	materialAsset->descriptorSet = materialDescriptorSet;

	return materialAsset;
}

void MaterialImporter::QueueReloadAsset(Uuid uuid) {
	auto& materialIterator = assets.find(uuid);
	if (materialIterator == assets.end()) {
		return;
	}

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;
	MaterialAsset* materialAsset = &materialIterator->second;

	std::string assetName, contentData;
	if (!assetManager->LoadFileText(AssetType::Mesh3d, uuid, assetName, contentData)) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "Could not find material by file.");
		return;
	}

	materialAsset->name = assetName;

	rapidjson::Document document;
	document.Parse(contentData.data());

	if (!document.HasMember("name")) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "No name found in material.");
		return;
	}
	const char* name = document["name"].GetString();

	if (!document.HasMember("shader")) {
		EngineCore::GetInstance().Print(LogSeverity::Error, "No shader found in material.");
		return;
	}

	Uuid shaderUuid(document["shader"].GetString());
	ShaderAsset* shaderAsset = assetManager->IncrementAssetUse<ShaderAsset>(shaderUuid);
	// TODO: Handle swapping between shaders

	if (materialAsset->buffer != nullptr) {
		delete materialAsset->buffer;
	}

	if (materialAsset->buffer != nullptr) {
		graphicsCore->DeleteDescriptorSet(materialAsset->descriptorSet);
	}

	if (materialAsset->buffer != nullptr) {
		graphicsCore->DeleteUniformBuffer(materialAsset->uniformBufferObject);
	}

	auto& reflectionData = shaderAsset->reflectionData;

	std::vector<DescriptorSet::Binding> bindings;
	SetupUniformBuffer(document, reflectionData, bindings, materialAsset->name, materialAsset);
	SetupSamplers(document, reflectionData, bindings);

	std::string descriptorSetName = (materialAsset->name + " Material Descriptor Set");

	GraphicsAPI::DescriptorSet::CreateInfo materialDescriptorSetCreateInfo{};
	materialDescriptorSetCreateInfo.debugName = descriptorSetName.c_str();
	materialDescriptorSetCreateInfo.bindings = bindings.data();
	materialDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	materialDescriptorSetCreateInfo.layout = shaderAsset->descriptorSetLayout;
	DescriptorSet* materialDescriptorSet = graphicsCore->CreateDescriptorSet(materialDescriptorSetCreateInfo);

	materialAsset->descriptorSet = materialDescriptorSet;
}

void MaterialImporter::SetupUniformBuffer(rapidjson::Document& document, ShaderReflectionData& reflectionData, std::vector<DescriptorSet::Binding>& bindings, std::string name, MaterialAsset* materialAsset) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
	char* bufferSpace = nullptr;

	ShaderReflectionData::StructData* materialUniformBuffer = nullptr;
	auto& uniformBuffers = reflectionData.uniformBuffers;
	for (auto& uniformBuffer : uniformBuffers) {
		if (uniformBuffer.name != "MaterialUbo") {
			continue;
		}

		materialUniformBuffer = &uniformBuffer;
	}

	if (materialUniformBuffer != nullptr) {
		std::string uniformBufferName = (name + " MaterialUbo");
		GraphicsAPI::UniformBuffer::CreateInfo ubCi{};
		ubCi.debugName = uniformBufferName.c_str();
		ubCi.isDynamic = true;
		ubCi.size = static_cast<uint32_t>(materialUniformBuffer->bufferSize);
		uniformBufferObject = graphicsCore->CreateUniformBuffer(ubCi);

		// TODO: Use materialUniformBuffer->bindingId
		DescriptorSet::Binding uniformBufferBinding{ uniformBufferObject };
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

	materialAsset->uniformBufferObject = uniformBufferObject;
	materialAsset->buffer = bufferSpace;
}

void MaterialImporter::SetupSamplers(rapidjson::Document& document, ShaderReflectionData& reflectionData, std::vector<DescriptorSet::Binding>& bindings) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;

	auto& textureReferencesFromMaterial = reflectionData.textures;
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

				TextureAsset* textureAsset = assetManager->IncrementAssetUse<TextureAsset>(textureUuid);
				// TODO: Use this to ensure it goes in the right place: textureReferencesFromMaterial[i].bindingId;
				Texture* itemPtr = textureAsset != nullptr
					? textureAsset->texture
					: missingTexture;
				DescriptorSet::Binding textureBinding{ itemPtr };
				bindings.push_back(textureBinding);
			}
			else {
				DescriptorSet::Binding textureBinding{ missingTexture };
				bindings.push_back(textureBinding);
			}
		}
	}
}
