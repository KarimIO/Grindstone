#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/Profiling.hpp>
#include <EngineCore/Logger.hpp>

#include "Loaders/AssetLoader.hpp"
#include "Loaders/ArchiveAssetLoader.hpp"

// Assets
#include "Materials/MaterialAsset.hpp"
#include "Materials/MaterialImporter.hpp"
#include "Shaders/ShaderAsset.hpp"
#include "Shaders/ShaderImporter.hpp"
#include "Textures/TextureAsset.hpp"
#include "Textures/TextureImporter.hpp"

#include "AssetManager.hpp"

using namespace Grindstone;
using namespace Grindstone::Assets;
using namespace Grindstone::Memory;

AssetManager::AssetManager(AssetLoader* assetLoader) {

	// TODO: Decide via preprocessor after we implement building the engine code during game build
	ownsAssetLoader = assetLoader == nullptr;
	this->assetLoader = ownsAssetLoader
		? static_cast<AssetLoader*>(AllocatorCore::Allocate<ArchiveAssetLoader>())
		: assetLoader;

	size_t count = static_cast<size_t>(AssetType::Count);
	assetTypeNames.resize(count);
	assetTypeImporters.resize(count);
	RegisterAssetType(AssetType::Undefined, "Undefined", nullptr);
	RegisterAssetType(ShaderAsset::GetStaticType(), "ShaderAsset", AllocatorCore::Allocate<ShaderImporter>());
	RegisterAssetType(TextureAsset::GetStaticType(), "TextureAsset", AllocatorCore::Allocate<TextureImporter>());
	RegisterAssetType(MaterialAsset::GetStaticType(), "MaterialAsset", AllocatorCore::Allocate<MaterialImporter>());
}

void AssetManager::UnregisterAssetType(AssetType assetType) {
	assetTypeImporters[static_cast<size_t>(assetType)] = nullptr;
	assetTypeNames[static_cast<size_t>(assetType)] = "";
}

AssetManager::~AssetManager() {
	if (ownsAssetLoader) {
		AllocatorCore::Free(assetLoader);
	}

	AllocatorCore::Free(static_cast<ShaderImporter*>(assetTypeImporters[static_cast<size_t>(ShaderAsset::GetStaticType())]));
	AllocatorCore::Free(static_cast<TextureImporter*>(assetTypeImporters[static_cast<size_t>(TextureAsset::GetStaticType())]));
	AllocatorCore::Free(static_cast<MaterialImporter*>(assetTypeImporters[static_cast<size_t>(MaterialAsset::GetStaticType())]));
	UnregisterAssetType(ShaderAsset::GetStaticType());
	UnregisterAssetType(TextureAsset::GetStaticType());
	UnregisterAssetType(MaterialAsset::GetStaticType());

	for (AssetImporter*& importer : assetTypeImporters) {
		if (importer != nullptr) {
			GPRINT_ERROR(LogSource::EngineCore, "Unfreed memory in AssetManager!");
			importer = nullptr;
		}
	}

	assetTypeImporters.clear();
}

AssetImporter* AssetManager::GetManager(AssetType assetType) {
	const size_t index = static_cast<size_t>(assetType);
	return assetTypeImporters[index];
}

void AssetManager::QueueReloadAsset(AssetType assetType, Uuid uuid) {
	std::scoped_lock lock(reloadMutex);
	queuedAssetReloads.emplace_back(assetType, uuid);
}

void AssetManager::ReloadQueuedAssets() {
	GRIND_PROFILE_SCOPE("AssetManager::ReloadQueuedAssets()");
	std::scoped_lock lock(reloadMutex);

	for (const auto& [assetType, uuid] : queuedAssetReloads) {
		const size_t assetTypeSizeT = static_cast<size_t>(assetType);
		if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
			return;
		}

		AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];

		if (assetImporter == nullptr) {
			return;
		}

		assetImporter->QueueReloadAsset(uuid);
	}

	queuedAssetReloads.clear();
}

void* AssetManager::GetAsset(AssetType assetType, std::string_view address) {
	if (address.empty()) {
		return nullptr;
	}

	const size_t assetTypeSizeT = static_cast<size_t>(assetType);
	if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
		return nullptr;
	}

	AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];

	void* loadedAsset = nullptr;
	if (assetImporter->TryGetIfLoaded(address, loadedAsset)) {
		return loadedAsset;
	}
	else {
		return assetTypeImporters[assetTypeSizeT]->ProcessLoadedFile(address);
	}
}

void* AssetManager::IncrementAssetUse(AssetType assetType, Uuid uuid) {
	const size_t assetTypeSizeT = static_cast<size_t>(assetType);
	if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
		return nullptr;
	}

	AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];
	return assetImporter->IncrementAssetUse(uuid);
}

void AssetManager::DecrementAssetUse(AssetType assetType, Uuid uuid) {
	const size_t assetTypeSizeT = static_cast<size_t>(assetType);
	if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
		return;
	}

	AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];
	assetImporter->DecrementAssetUse(uuid);
}

void* AssetManager::GetAsset(AssetType assetType, Uuid uuid) {
	if (!uuid.IsValid()) {
		return nullptr;
	}

	const size_t assetTypeSizeT = static_cast<size_t>(assetType);
	if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
		return nullptr;
	}

	AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];

	void* loadedAsset = nullptr;
	if (assetImporter->TryGetIfLoaded(uuid, loadedAsset)) {
		return loadedAsset;
	}
	else {
		return assetTypeImporters[assetTypeSizeT]->ProcessLoadedFile(uuid);
	}
}

AssetLoadBinaryResult AssetManager::LoadBinaryByPath(AssetType assetType, const std::filesystem::path& path) {
	return assetLoader->LoadBinaryByPath(assetType, path);
}

AssetLoadBinaryResult AssetManager::LoadBinaryByAddress(AssetType assetType, std::string_view address) {
	return assetLoader->LoadBinaryByAddress(assetType, address);
}

AssetLoadBinaryResult AssetManager::LoadBinaryByUuid(AssetType assetType, Uuid uuid) {
	return assetLoader->LoadBinaryByUuid(assetType, uuid);
}

AssetLoadTextResult AssetManager::LoadTextByPath(AssetType assetType, const std::filesystem::path& path) {
	return assetLoader->LoadTextByPath(assetType, path);
}

AssetLoadTextResult AssetManager::LoadTextByAddress(AssetType assetType, std::string_view address) {
	return assetLoader->LoadTextByAddress(assetType, address);
}

AssetLoadTextResult AssetManager::LoadTextByUuid(AssetType assetType, Uuid uuid) {
	return assetLoader->LoadTextByUuid(assetType, uuid);
}

bool AssetManager::LoadShaderSetByUuid(
	Uuid uuid,
	uint8_t shaderStagesBitMask,
	size_t numShaderStages,
	std::vector<Grindstone::GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData>& shaderStageCreateInfos,
	std::vector<std::vector<char>>& fileData
) {
	shaderStageCreateInfos.resize(numShaderStages);
	fileData.resize(numShaderStages);

	uint8_t shaderStagesBitMaskAsUint = static_cast<uint8_t>(shaderStagesBitMask);

	size_t shaderIterator = 0;
	for (
		ShaderStage stage = ShaderStage::Vertex;
		stage < ShaderStage::GraphicsCount;
		stage = static_cast<ShaderStage>(static_cast<uint8_t>(stage) + 1)
	) {
		const uint8_t stageBit = (1 << static_cast<uint8_t>(stage));
		if ((stageBit & shaderStagesBitMaskAsUint) != stageBit) {
			continue;
		}

		if (!assetLoader->LoadShaderStageByUuid(uuid, stage, shaderStageCreateInfos[shaderIterator], fileData[shaderIterator])) {
			return false;
		}

		++shaderIterator;
	}

	return true;
}

bool AssetManager::LoadShaderStageByUuid(Uuid uuid, GraphicsAPI::ShaderStage shaderStage, GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo, std::vector<char>& fileData) {
	return assetLoader->LoadShaderStageByUuid(uuid, shaderStage, stageCreateInfo, fileData);
}

bool AssetManager::LoadShaderSetByAddress(
	std::string_view address,
	uint8_t shaderStagesBitMask,
	size_t numShaderStages,
	std::vector<GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData>& shaderStageCreateInfos,
	std::vector<std::vector<char>>& fileData
) {
	shaderStageCreateInfos.resize(numShaderStages);
	fileData.resize(numShaderStages);

	uint8_t shaderStagesBitMaskAsUint = static_cast<uint8_t>(shaderStagesBitMask);

	size_t shaderIterator = 0;
	for (
		ShaderStage stage = ShaderStage::Vertex;
		stage < ShaderStage::GraphicsCount;
		stage = static_cast<ShaderStage>(static_cast<uint8_t>(stage) + 1)
	) {
		const uint8_t stageBit = (1 << static_cast<uint8_t>(stage));
		if ((stageBit & shaderStagesBitMaskAsUint) != stageBit) {
			continue;
		}

		if (!assetLoader->LoadShaderStageByAddress(address, stage, shaderStageCreateInfos[shaderIterator], fileData[shaderIterator])) {
			return false;
		}

		++shaderIterator;
	}

	return true;
}

bool AssetManager::LoadShaderStageByAddress(
	std::string_view address,
	GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	return assetLoader->LoadShaderStageByAddress(address, shaderStage, stageCreateInfo, fileData);
}

bool AssetManager::LoadShaderSetByPath(
	const std::filesystem::path& path,
	uint8_t shaderStagesBitMask,
	size_t numShaderStages,
	std::vector<GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData>& shaderStageCreateInfos,
	std::vector<std::vector<char>>& fileData
) {
	shaderStageCreateInfos.resize(numShaderStages);
	fileData.resize(numShaderStages);

	uint8_t shaderStagesBitMaskAsUint = static_cast<uint8_t>(shaderStagesBitMask);

	size_t shaderIterator = 0;
	for (
		ShaderStage stage = ShaderStage::Vertex;
		stage < ShaderStage::GraphicsCount;
		stage = static_cast<ShaderStage>(static_cast<uint8_t>(stage) + 1)
	) {
		const uint8_t stageBit = (1 << static_cast<uint8_t>(stage));
		if ((stageBit & shaderStagesBitMaskAsUint) != stageBit) {
			continue;
		}

		if (!assetLoader->LoadShaderStageByPath(path, stage, shaderStageCreateInfos[shaderIterator], fileData[shaderIterator])) {
			return false;
		}

		++shaderIterator;
	}

	return true;
}

bool AssetManager::LoadShaderStageByPath(
	const std::filesystem::path& path,
	GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	return assetLoader->LoadShaderStageByPath(path, shaderStage, stageCreateInfo, fileData);
}

std::string& AssetManager::GetTypeName(AssetType assetType) {
	return assetTypeNames[static_cast<size_t>(assetType)];
}

void AssetManager::RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* importer) {
	assetTypeNames[static_cast<size_t>(assetType)] = typeName;
	assetTypeImporters[static_cast<size_t>(assetType)] = importer;
}
