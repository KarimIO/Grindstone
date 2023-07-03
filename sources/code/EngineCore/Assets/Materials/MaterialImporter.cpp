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
		GraphicsAPI::UniformBuffer::CreateInfo ubCi{};
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
		}
	}


	auto& textureReferencesFromMaterial = shaderAsset->reflectionData.textures;
	bool hasSamplers = document.HasMember("samplers");
	if (textureReferencesFromMaterial.size() > 0 && hasSamplers) {
		auto& samplersJson = document["samplers"];
		std::vector<GraphicsAPI::Texture*> textures;
		textures.resize(textureReferencesFromMaterial.size());
		Texture* validTexture = nullptr;
		for (size_t i = 0; i < textureReferencesFromMaterial.size(); ++i) {
			const char* textureName = textureReferencesFromMaterial[i].name.c_str();
			if (samplersJson.HasMember(textureName)) {
				const char* textureUuidAsString = samplersJson[textureName].GetString();
				Uuid textureUuid(textureUuidAsString);

				// TODO: Handle if texture isn't set
				TextureAsset* textureAsset = assetManager->GetAsset<TextureAsset>(textureUuid);
				DescriptorSet::Binding textureBinding{};
				textureBinding.bindingIndex = textureReferencesFromMaterial[i].bindingId;
				textureBinding.bindingType = BindingType::Texture;
				textureBinding.itemPtr = textureAsset != nullptr
					? textureAsset->texture
					: validTexture;
				textureBinding.count = 1;
				bindings.push_back(textureBinding);

				if (textureAsset != nullptr) {
					validTexture = textureAsset->texture;
				}
			}
			else {
				DescriptorSet::Binding textureBinding{};
				textureBinding.bindingIndex = textureReferencesFromMaterial[i].bindingId;
				textureBinding.bindingType = BindingType::Texture;
				textureBinding.itemPtr = validTexture;
				textureBinding.count = 1;
				bindings.push_back(textureBinding);
			}
		}
	}

	GraphicsAPI::DescriptorSet::CreateInfo materialDescriptorSetCreateInfo{};
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

#if 0
Material& MaterialImporter::LoadMaterial(BaseAssetRenderer* assetRenderer, const char* path) {
	try {
		Material* material = nullptr;
		if (!TryGetMaterial(path, material)) {
			material = &CreateMaterialFromFile(assetRenderer, path);
		}

		return *material;
	}
	catch (std::runtime_error& e) {
		EngineCore::GetInstance().Print(LogSeverity::Error, e.what());
		return *assetRenderer->GetErrorMaterial();
	}
}

void MaterialImporter::ReloadMaterialIfLoaded(const char* path) {}

void MaterialImporter::RemoveRenderableFromMaterial(std::string uuid, ECS::Entity entity, void* renderable) {
	auto materialInMap = materials.find(uuid);
	if (materialInMap != materials.end()) {
		auto material = &materialInMap->second;

		size_t size = material->renderables.size();
		size_t numItemsToDelete = 0;
		for (size_t i = 0; i < size - numItemsToDelete; ++i) {
			auto& renderablePair = material->renderables[i];
			if (
				renderablePair.first == entity &&
				renderablePair.second == renderable
			) {
				++numItemsToDelete;
				material->renderables[i] = material->renderables[size - numItemsToDelete];
			}
		}

		if (numItemsToDelete > 0) {
			material->renderables.erase(material->renderables.end() - numItemsToDelete);
		}

		if (material->renderables.size() == 0) {
			ShaderImporter* shaderImporter = EngineCore::GetInstance().shaderImporter;
			shaderImporter->RemoveMaterialFromShader(material->shader, material);

			materials.erase(materialInMap);
		}
	}
}

bool MaterialImporter::TryGetMaterial(const char* path, Material*& material) {
	auto materialInMap = materials.find(path);
	if (materialInMap != materials.end()) {
		material = &materialInMap->second;
		return true;
	}

	return false;
}

void MaterialImporter::CreateMaterialFromData(
	std::filesystem::path relativePath,
	Material& material,
	BaseAssetRenderer* assetRenderer,
	const char* data
) {
}

Material& MaterialImporter::CreateMaterialFromFile(BaseAssetRenderer* assetRenderer, const char* path) {
	std::filesystem::path completePath = EngineCore::GetInstance().GetAssetPath(path);
	if (!std::filesystem::exists(completePath)) {
		throw std::runtime_error(completePath.string() + " material doesn't exist.");
	}

	std::filesystem::path parentDirectory = completePath.parent_path();
	std::string fileContent = Utils::LoadFileText(completePath.string().c_str());
	Material& material = materials[path];
	material.uuid = Uuid(path);
	CreateMaterialFromData(parentDirectory, material, assetRenderer, fileContent.c_str());

	return material;
}
#endif
