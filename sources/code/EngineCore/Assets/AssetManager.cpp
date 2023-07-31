#include "AssetManager.hpp"
#include "Loaders/AssetLoader.hpp"
#include "Loaders/FileAssetLoader.hpp"

// Assets
#include "Materials/MaterialAsset.hpp"
#include "Materials/MaterialImporter.hpp"
#include "Shaders/ShaderAsset.hpp"
#include "Shaders/ShaderImporter.hpp"
#include "Textures/TextureAsset.hpp"
#include "Textures/TextureImporter.hpp"
#include "Shaders/ShaderAsset.hpp"

using namespace Grindstone;
using namespace Grindstone::Assets;

AssetManager::AssetManager() {
	assetLoader = new FileAssetLoader();

	size_t count = static_cast<size_t>(AssetType::Count);
	assetTypeNames.resize(count);
	assetTypeImporters.resize(count);
	RegisterAssetType(AssetType::Undefined, "Undefined", nullptr);
	RegisterAssetType(ShaderAsset::GetStaticType(), "ShaderAsset", new ShaderImporter());
	RegisterAssetType(TextureAsset::GetStaticType(), "TextureAsset", new TextureImporter());
	RegisterAssetType(MaterialAsset::GetStaticType(), "MaterialAsset", new MaterialImporter());
}

void AssetManager::QueueReloadAsset(AssetType assetType, Uuid uuid) {
	std::scoped_lock lock(reloadMutex);
	queuedAssetReloads.emplace_back(assetType, uuid);
}

void AssetManager::ReloadQueuedAssets() {
	std::scoped_lock lock(reloadMutex);

	for (auto& assetSet : queuedAssetReloads) {
		size_t assetTypeSizeT = static_cast<size_t>(assetSet.first);
		if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
			return;
		}

		AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];
		assetImporter->QueueReloadAsset(assetSet.second);
	}

	queuedAssetReloads.clear();
}

void* AssetManager::GetAsset(AssetType assetType, const char* path) {
	size_t assetTypeSizeT = static_cast<size_t>(assetType);
	if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
		return nullptr;
	}

	AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];

	void* loadedAsset = nullptr;
	if (assetImporter->TryGetIfLoaded(path, loadedAsset)) {
		return loadedAsset;
	}
	else {
		return assetTypeImporters[assetTypeSizeT]->ProcessLoadedFile(path);
	}
}

void* AssetManager::GetAsset(AssetType assetType, Uuid uuid) {
	size_t assetTypeSizeT = static_cast<size_t>(assetType);
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

bool AssetManager::LoadFile(const char* path, char*& fileData, size_t& fileSize) {
	fileData = nullptr;
	fileSize = 0;

	assetLoader->Load(std::filesystem::path(path), fileData, fileSize);
	return fileData != nullptr;
}

// Loads an actual file, not an asset. The file still needs to be imported and handled.
// Make loading indirect to conceal how files are loading from asset importers
// This allows multiple files to be imported per asset, and allows different types of loading depending on the
// asset, such as shaders loading from a file if it's not compiled yet, or loading a compiled shader if it is loaded
bool AssetManager::LoadFile(Uuid uuid, char*& fileData, size_t& fileSize) {
	fileData = nullptr;
	fileSize = 0;

	assetLoader->Load(uuid, fileData, fileSize);
	return fileData != nullptr;
}

// Loads an actual file, not an asset. The file still needs to be imported and handled.
// Make loading indirect to conceal how files are loading from asset importers
// This allows multiple files to be imported per asset, and allows different types of loading depending on the
// asset, such as shaders loading from a file if it's not compiled yet, or loading a compiled shader if it is loaded
bool AssetManager::LoadFileText(Uuid uuid, std::string& fileData) {
	return assetLoader->LoadText(uuid, fileData);
}

bool AssetManager::LoadShaderSet(
	Uuid uuid,
	uint8_t shaderStagesBitMask,
	size_t numShaderStages,
	std::vector<ShaderStageCreateInfo>& shaderStageCreateInfos,
	std::vector<std::vector<char>>& fileData
) {
	shaderStageCreateInfos.resize(numShaderStages);
	fileData.resize(numShaderStages);

	uint8_t shaderStagesBitMaskAsUint = static_cast<uint8_t>(shaderStagesBitMask);

	size_t shaderIterator = 0;
	for (
		ShaderStage stage = ShaderStage::Vertex;
		stage < ShaderStage::Compute;
		stage = (ShaderStage)((uint8_t)stage + 1)
	) {
		uint8_t stageBit = (1 << (uint8_t)stage);
		if ((stageBit & shaderStagesBitMaskAsUint) != stageBit) {
			continue;
		}

		if (!assetLoader->LoadShaderStage(uuid, stage, shaderStageCreateInfos[shaderIterator], fileData[shaderIterator])) {
			return false;
		}

		++shaderIterator;
	}

	return true;
}

bool AssetManager::LoadShaderStage(Uuid uuid, GraphicsAPI::ShaderStage shaderStage, GraphicsAPI::ShaderStageCreateInfo& stageCreateInfo, std::vector<char>& fileData) {
	return assetLoader->LoadShaderStage(uuid, shaderStage, stageCreateInfo, fileData);
}

std::string& AssetManager::GetTypeName(AssetType assetType) {
	return assetTypeNames[static_cast<size_t>(assetType)];
}

void AssetManager::RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* importer) {
	assetTypeNames[static_cast<size_t>(assetType)] = typeName;
	assetTypeImporters[static_cast<size_t>(assetType)] = importer;
}
