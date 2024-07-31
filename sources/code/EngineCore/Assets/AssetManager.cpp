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

void* AssetManager::GetAsset(AssetType assetType, const char* path) {
	if (path == nullptr) {
		return nullptr;
	}

	const size_t assetTypeSizeT = static_cast<size_t>(assetType);
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

bool AssetManager::LoadFile(AssetType assetType, const char* path, std::string& assetName, char*& fileData, size_t& fileSize) {
	fileData = nullptr;
	fileSize = 0;

	assetLoader->Load(assetType, std::filesystem::path(path), assetName, fileData, fileSize);
	return fileData != nullptr;
}

// Loads an actual file, not an asset. The file still needs to be imported and handled.
// Make loading indirect to conceal how files are loading from asset importers
// This allows multiple files to be imported per asset, and allows different types of loading depending on the
// asset, such as shaders loading from a file if it's not compiled yet, or loading a compiled shader if it is loaded
bool AssetManager::LoadFile(AssetType assetType, Uuid uuid, std::string& assetName, char*& fileData, size_t& fileSize) {
	fileData = nullptr;
	fileSize = 0;

	assetLoader->Load(assetType, uuid, assetName, fileData, fileSize);
	return fileData != nullptr;
}

// Loads an actual file, not an asset. The file still needs to be imported and handled.
// Make loading indirect to conceal how files are loading from asset importers
// This allows multiple files to be imported per asset, and allows different types of loading depending on the
// asset, such as shaders loading from a file if it's not compiled yet, or loading a compiled shader if it is loaded
bool AssetManager::LoadFileText(AssetType assetType, Uuid uuid, std::string& assetName, std::string& fileData) {
	return assetLoader->LoadText(assetType, uuid, assetName, fileData);
}

bool AssetManager::LoadFileText(AssetType assetType, std::filesystem::path path, std::string& assetName, std::string& fileData) {
	return assetLoader->LoadText(assetType, path, assetName, fileData);
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
		stage = static_cast<ShaderStage>(static_cast<uint8_t>(stage) + 1)
	) {
		const uint8_t stageBit = (1 << static_cast<uint8_t>(stage));
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
