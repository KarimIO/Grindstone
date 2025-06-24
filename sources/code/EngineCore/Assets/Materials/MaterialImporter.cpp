#include <rapidjson/document.h>

#include <EngineCore/Assets/PipelineSet/GraphicsPipelineImporter.hpp>
#include <EngineCore/Assets/Textures/TextureImporter.hpp>
#include <EngineCore/AssetRenderer/BaseAssetRenderer.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/Logger.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/Image.hpp>

#include "MaterialImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;

template<typename Type, typename JsonType>
static inline void ReadMaterialDataArray(
	Grindstone::Buffer& buffer,
	const rapidjson::Value& materialDocumentData,
	const PipelineAssetMetaData::Parameter& shaderMemberData,
	JsonType(rapidjson::Value::* memberFunction)() const
) {
	// TODO: Maybe investigate using stride for alignment
	// TODO: Verify array element count
	std::vector<Type> materialArray;
	materialArray.resize(materialDocumentData.Size());

	for (rapidjson::SizeType i = 0; i < materialDocumentData.Size(); ++i) {
		if (materialDocumentData[i].GetType() != rapidjson::kNumberType) {
			GPRINT_ERROR_V(Grindstone::LogSource::EngineCore, "Expected a number for an element of the array '{}' in material '{}'!", shaderMemberData.name, "Unset material name");
		}
		else {
			materialArray[i] = static_cast<Type>((materialDocumentData[i].*memberFunction)());
		}
	}

	memcpy(buffer.Get() + shaderMemberData.offset, materialArray.data(), sizeof(Type) * materialArray.size());
}

static void ReadMaterialDataMember(
	const std::string& name,
	Grindstone::Buffer& buffer,
	const rapidjson::Value& materialDocumentData,
	const PipelineAssetMetaData::Parameter& shaderMemberData
) {
	// TODO: Verify that the material data type matches the source shader data type

	switch (materialDocumentData.GetType()) {
		case rapidjson::kNullType:
			GPRINT_ERROR_V(Grindstone::LogSource::EngineCore, "Unsupported type 'null' for member '{}' in material '{}'!", shaderMemberData.name, name);
			break;
		case rapidjson::kFalseType: {
			bool value = false;
			memcpy(buffer.Get() + shaderMemberData.offset, &value, shaderMemberData.size);
			break;
		}
		case rapidjson::kTrueType: {
			bool value = true;
			memcpy(buffer.Get() + shaderMemberData.offset, &value, shaderMemberData.size);
			break;
		}
		case rapidjson::kObjectType:
			// TODO: We need to handle this in the future.
			GPRINT_ERROR_V(Grindstone::LogSource::EngineCore, "Unhandled type 'object' for member '{}' in material '{}'! We will support this in the future.", shaderMemberData.name, name);
			break;
		case rapidjson::kArrayType: {
			switch (shaderMemberData.type) {
			default:
				GPRINT_ERROR_V(Grindstone::LogSource::EngineCore, "Unhandled shader member type for array '{}' in material '{}'!", shaderMemberData.name, name);
				break;
			case PipelineAssetMetaData::ParameterType::Float:
				ReadMaterialDataArray<float>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetFloat);
				break;
			case PipelineAssetMetaData::ParameterType::Double:
				ReadMaterialDataArray<double>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetDouble);
				break;
			case PipelineAssetMetaData::ParameterType::SByte:
				ReadMaterialDataArray<int8_t>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetInt);
				break;
			case PipelineAssetMetaData::ParameterType::UByte:
				ReadMaterialDataArray<uint8_t>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetUint);
				break;
			case PipelineAssetMetaData::ParameterType::Short:
				ReadMaterialDataArray<int16_t>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetInt);
				break;
			case PipelineAssetMetaData::ParameterType::UShort:
				ReadMaterialDataArray<uint16_t>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetUint);
				break;
			case PipelineAssetMetaData::ParameterType::Int:
				ReadMaterialDataArray<int32_t>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetInt);
				break;
			case PipelineAssetMetaData::ParameterType::UInt:
				ReadMaterialDataArray<uint32_t>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetUint);
				break;
			case PipelineAssetMetaData::ParameterType::Int64:
				ReadMaterialDataArray<int64_t>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetInt64);
				break;
			case PipelineAssetMetaData::ParameterType::UInt64:
				ReadMaterialDataArray<uint64_t>(buffer, materialDocumentData, shaderMemberData, &rapidjson::Value::GetUint64);
				break;
			}
			break;
		}
		case rapidjson::kStringType: {
			// Shader languages don't support strings.
			GPRINT_ERROR_V(Grindstone::LogSource::EngineCore, "Unsupported type 'string' for member '{}' in material '{}'! Shaders do not support strings.", shaderMemberData.name, name);
			break;
		}
		case rapidjson::kNumberType: {
			void* offset = buffer.Get() + shaderMemberData.offset;
			switch (shaderMemberData.type) {
				default:
					GPRINT_ERROR_V(Grindstone::LogSource::EngineCore, "Unhandled shader member type for member '{}' in material '{}'!", shaderMemberData.name, name);
					break;
				case PipelineAssetMetaData::ParameterType::Float: {
					float value = materialDocumentData.GetFloat();
					memcpy(offset, &value, sizeof(value));
					break;
				}
				case PipelineAssetMetaData::ParameterType::Double: {
					double value = materialDocumentData.GetDouble();
					memcpy(offset, &value, sizeof(value));
					break;
				}
				case PipelineAssetMetaData::ParameterType::SByte: {
					int8_t value = static_cast<int8_t>(materialDocumentData.GetInt());
					memcpy(offset, &value, sizeof(value));
					break;
				}
				case PipelineAssetMetaData::ParameterType::UByte: {
					uint8_t value = static_cast<uint8_t>(materialDocumentData.GetUint());
					memcpy(offset, &value, sizeof(value));
					break;
				}
				case PipelineAssetMetaData::ParameterType::Short: {
					int16_t value = static_cast<int16_t>(materialDocumentData.GetInt());
					memcpy(offset, &value, sizeof(value));
					break;
				}
				case PipelineAssetMetaData::ParameterType::UShort: {
					uint16_t value = static_cast<uint16_t>(materialDocumentData.GetUint());
					memcpy(offset, &value, sizeof(value));
					break;
				}
				case PipelineAssetMetaData::ParameterType::Int: {
					int32_t value = materialDocumentData.GetInt();
					memcpy(offset, &value, sizeof(value));
					break;
				}
				case PipelineAssetMetaData::ParameterType::UInt: {
					uint32_t value = materialDocumentData.GetUint();
					memcpy(offset, &value, sizeof(value));
					break;
				}
				case PipelineAssetMetaData::ParameterType::Int64: {
					int64_t value = materialDocumentData.GetInt64();
					memcpy(offset, &value, sizeof(value));
					break;
				}
				case PipelineAssetMetaData::ParameterType::UInt64: {
					uint64_t value = materialDocumentData.GetUint64();
					memcpy(offset, &value, sizeof(value));
					break;
				}
			}
			break;
		}
	}
}

static void SetupUniformBuffer(
	const rapidjson::Document& document,
	Grindstone::GraphicsPipelineAsset& pipelineSetAsset,
	std::vector<GraphicsAPI::DescriptorSet::Binding>& bindings,
	const std::string& name,
	Grindstone::MaterialAsset& materialAsset
) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	GraphicsAPI::Buffer* uniformBufferObject = nullptr;

	const Grindstone::PipelineAssetMetaData::Buffer* materialBuffer =
		pipelineSetAsset.GetBufferMetaData();

	if (materialBuffer != nullptr) {
		if (materialBuffer->bufferSize > 0) {
			materialAsset.materialDataBuffer = Grindstone::Buffer(materialBuffer->bufferSize);

			const rapidjson::Value& materialDocumentParametersJson = document["parameters"];
			for (const PipelineAssetMetaData::Parameter& shaderMemberData : materialBuffer->parameters) {
				const auto& materialDocumentParamIterator = materialDocumentParametersJson.FindMember(shaderMemberData.name.c_str());
				if (materialDocumentParamIterator != materialDocumentParametersJson.MemberEnd()) {
					const rapidjson::Value& materialDocumentData = materialDocumentParamIterator->value;
					ReadMaterialDataMember(
						name, materialAsset.materialDataBuffer, materialDocumentData, shaderMemberData
					);
				}
				// TODO: Handle default data from shaders on else here.
			}
		}

		std::string uniformBufferName = (name + " MaterialUbo");
		GraphicsAPI::Buffer::CreateInfo ubCi{};
		ubCi.debugName = uniformBufferName.c_str();
		ubCi.content = materialAsset.materialDataBuffer.Get();
		ubCi.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform;
		ubCi.memoryUsage = GraphicsAPI::MemUsage::CPUToGPU;
		ubCi.bufferSize = static_cast<uint32_t>(materialBuffer->bufferSize);
		uniformBufferObject = graphicsCore->CreateBuffer(ubCi);

		GraphicsAPI::DescriptorSet::Binding uniformBufferBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( uniformBufferObject );
		bindings.push_back(uniformBufferBinding);
	}

	materialAsset.materialDataUniformBuffer = uniformBufferObject;
}

static void SetupSamplers(
	const rapidjson::Document& document,
	Grindstone::GraphicsPipelineAsset& pipelineSetAsset,
	Grindstone::GraphicsAPI::Image* missingTexture,
	std::vector<GraphicsAPI::DescriptorSet::Binding>& bindings,
	Grindstone::MaterialAsset& materialAsset
) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;
	size_t textureCount = pipelineSetAsset.GetTextureMetaDataSize();
	materialAsset.textures.resize(textureCount);

	if (textureCount > 0 && document.HasMember("samplers")) {
		const auto& resourcesJson = document["samplers"];

		std::vector<GraphicsAPI::Image*> textures;
		textures.resize(textureCount);
		for (size_t i = 0; i < textureCount; ++i) {
			const Grindstone::PipelineAssetMetaData::ResourceSlot& resourceMetaData =
				pipelineSetAsset.GetTextureMetaDataByIndex(i);
			const char* textureName = resourceMetaData.slotName.c_str();

			if (resourcesJson.HasMember(textureName)) {
				GraphicsAPI::Image* itemPtr = missingTexture;
				if (!resourcesJson[textureName].IsString()) {
					GPRINT_ERROR_V(LogSource::EngineCore, "Textures expects a UUID in the form of a string in member {} of material {}.", textureName, materialAsset.name.c_str());
				}
				else {
					const char* textureValueString = resourcesJson[textureName].GetString();

					Grindstone::AssetReference<TextureAsset> textureReference;

					Grindstone::Uuid textureUuid;
					if (Grindstone::Uuid::MakeFromString(textureValueString, textureUuid)) {
						textureReference = assetManager->GetAssetReferenceByUuid<TextureAsset>(textureUuid);
					}
					else {
						textureReference = assetManager->GetAssetReferenceByAddress<TextureAsset>(textureValueString);
					}

					// TODO: Use this to ensure it goes in the right place: textureReferencesFromMaterial[i].bindingId;
					if (textureReference.IsValid()) {
						materialAsset.textures[i] = textureReference;
						const TextureAsset* textureAsset = textureReference.Get();
						itemPtr = textureAsset != nullptr
							? textureAsset->image
							: nullptr;
					}
				}

				GraphicsAPI::DescriptorSet::Binding textureBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( itemPtr );
				bindings.push_back(textureBinding);
			}
			else {
				GraphicsAPI::DescriptorSet::Binding textureBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( missingTexture );
				bindings.push_back(textureBinding);
			}
		}
	}
}

MaterialImporter::MaterialImporter() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	std::array<unsigned char, 16> colorData = {
		0x00, 0x00, 0x00, 0xff,
		0x00, 0x00, 0x00, 0xff,
		0x00, 0x00, 0x00, 0xff,
		0x00, 0x00, 0x00, 0xff,
	};

	GraphicsAPI::Image::CreateInfo blackTextureCreateInfo{};
	blackTextureCreateInfo.debugName = "Black Missing Texture";
	blackTextureCreateInfo.initialData = reinterpret_cast<const char*>(&colorData);
	blackTextureCreateInfo.initialDataSize = static_cast<uint32_t>(colorData.size());
	blackTextureCreateInfo.width = 2;
	blackTextureCreateInfo.height = 2;
	blackTextureCreateInfo.format = GraphicsAPI::Format::R8G8B8A8_UNORM;
	blackTextureCreateInfo.mipLevels = 1;
	blackTextureCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::TransferSrc |
		GraphicsAPI::ImageUsageFlags::TransferDst |
		GraphicsAPI::ImageUsageFlags::Sampled;
	missingTexture = graphicsCore->CreateImage(blackTextureCreateInfo);
}

static bool LoadMaterial(
	Grindstone::MaterialAsset& material,
	const std::string& displayName,
	Grindstone::GraphicsAPI::Image* missingTexture,
	const std::string_view materialContent
) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;

	rapidjson::Document document;
	if (document.Parse(materialContent.data()).HasParseError()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Unable to parse material {}.", displayName);
		material.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	Grindstone::Uuid shaderUuid;
	if (!document.HasMember("shader") || !Grindstone::Uuid::MakeFromString(document["shader"].GetString(), shaderUuid)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "No shader found in material {}.", displayName);
		material.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	if (shaderUuid != material.pipelineSetAsset.uuid) {
		material.pipelineSetAsset = assetManager->GetAssetReferenceByUuid<GraphicsPipelineAsset>(shaderUuid);
	}

	if (!material.pipelineSetAsset.IsValid()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Failed to load shader for {}.", displayName);
		material.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	Grindstone::GraphicsPipelineAsset* pipelineSetAsset = material.pipelineSetAsset.Get();
	if (pipelineSetAsset == nullptr) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Failed to load shader {} for material {}.", material.pipelineSetAsset.uuid.ToString(), displayName);
		material.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	Grindstone::GraphicsAPI::Sampler::CreateInfo samplerCreateInfo{};
	samplerCreateInfo.debugName = "Material Sampler";
	samplerCreateInfo.options.anistropy = 16.0f;
	samplerCreateInfo.options.mipMin = -1000.0f;
	samplerCreateInfo.options.mipMax = 1000.0f;
	samplerCreateInfo.options.mipFilter = GraphicsAPI::TextureFilter::Linear;
	samplerCreateInfo.options.minFilter = GraphicsAPI::TextureFilter::Linear;
	samplerCreateInfo.options.magFilter = GraphicsAPI::TextureFilter::Linear;
	samplerCreateInfo.options.wrapModeU = GraphicsAPI::TextureWrapMode::Repeat;
	samplerCreateInfo.options.wrapModeV = GraphicsAPI::TextureWrapMode::Repeat;
	samplerCreateInfo.options.wrapModeW = GraphicsAPI::TextureWrapMode::Repeat;

	std::vector<GraphicsAPI::DescriptorSet::Binding> bindings;
	bindings.emplace_back(GraphicsAPI::DescriptorSet::Binding::Sampler(graphicsCore->CreateSampler(samplerCreateInfo)));
	// SetupUniformBuffer(document, *pipelineSetAsset, bindings, displayName, material);
	SetupSamplers(document, *pipelineSetAsset, missingTexture, bindings, material);

	std::string descriptorSetName = (displayName + " Material Descriptor Set");

	GraphicsAPI::DescriptorSet::CreateInfo materialDescriptorSetCreateInfo{};
	materialDescriptorSetCreateInfo.debugName = descriptorSetName.c_str();
	materialDescriptorSetCreateInfo.bindings = bindings.data();
	materialDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	materialDescriptorSetCreateInfo.layout = pipelineSetAsset->GetMaterialDescriptorLayout();
	GraphicsAPI::DescriptorSet* materialDescriptorSet = graphicsCore->CreateDescriptorSet(materialDescriptorSetCreateInfo);

	material.materialDescriptorSet = materialDescriptorSet;
	material.assetLoadStatus = AssetLoadStatus::Ready;
	return true;
}

void* MaterialImporter::LoadAsset(Uuid uuid) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;

	auto& materialIterator = assets.emplace(uuid, Grindstone::MaterialAsset(uuid));
	Grindstone::MaterialAsset& materialAsset = materialIterator.first->second;

	Assets::AssetLoadTextResult result = assetManager->LoadTextByUuid(AssetType::Material, uuid);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not find material with id {}.", uuid.ToString());
		materialAsset.assetLoadStatus = AssetLoadStatus::Missing;
		return nullptr;
	}

	materialAsset.name = result.displayName;
	materialAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!LoadMaterial(materialAsset, result.displayName, missingTexture, result.content)) {
		return nullptr;
	}

	return &materialAsset;
}

void MaterialImporter::QueueReloadAsset(Uuid uuid) {
	auto& materialIterator = assets.find(uuid);
	if (materialIterator == assets.end()) {
		return;
	}

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;
	MaterialAsset& materialAsset = materialIterator->second;

	Assets::AssetLoadTextResult result = assetManager->LoadTextByUuid(AssetType::Material, uuid);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not find material with id {}.", uuid.ToString());
		return;
	}

	if (materialAsset.materialDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(materialAsset.materialDescriptorSet);
	}

	if (materialAsset.materialDataUniformBuffer != nullptr) {
		graphicsCore->DeleteBuffer(materialAsset.materialDataUniformBuffer);
	}

	materialAsset.name = result.displayName;
	materialAsset.assetLoadStatus = AssetLoadStatus::Reloading;

	LoadMaterial(materialAsset, result.displayName, missingTexture, result.content);
}

MaterialImporter::~MaterialImporter() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	for (auto& asset : assets) {
		graphicsCore->DeleteDescriptorSet(asset.second.materialDescriptorSet);
		graphicsCore->DeleteBuffer(asset.second.materialDataUniformBuffer);
	}

	assets.clear();

	graphicsCore->DeleteImage(missingTexture);
}
