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
	RegisterAssetType<ShaderImporter>();
	RegisterAssetType<TextureImporter>();
	RegisterAssetType<MaterialImporter>();
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

void* AssetManager::GetAssetByUuid(AssetType assetType, Uuid uuid) {
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
		return assetTypeImporters[assetTypeSizeT]->LoadAsset(uuid);
	}
}

Grindstone::Uuid AssetManager::GetUuidByAddress(AssetType assetType, std::string_view address) {
	return assetLoader->GetUuidByAddress(assetType, address);
}

AssetLoadBinaryResult AssetManager::LoadBinaryByUuid(AssetType assetType, Uuid uuid) {
	return assetLoader->LoadBinaryByUuid(assetType, uuid);
}

AssetLoadTextResult AssetManager::LoadTextByUuid(AssetType assetType, Uuid uuid) {
	return assetLoader->LoadTextByUuid(assetType, uuid);
}

void* AssetManager::GetAndIncrementAssetCount(Grindstone::AssetType assetType, Grindstone::Uuid uuid) {
	const size_t assetTypeSizeT = static_cast<size_t>(assetType);
	if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
		return nullptr;
	}

	AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];
	return assetImporter->IncrementAssetUse(uuid);
}

void AssetManager::IncrementAssetCount(Grindstone::AssetType assetType, Grindstone::Uuid uuid) {
	const size_t assetTypeSizeT = static_cast<size_t>(assetType);
	if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
		return;
	}

	AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];
	assetImporter->IncrementAssetUse(uuid);
}

void AssetManager::DecrementAssetCount(Grindstone::AssetType assetType, Grindstone::Uuid uuid) {
	const size_t assetTypeSizeT = static_cast<size_t>(assetType);
	if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
		return;
	}

	AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];
	assetImporter->DecrementAssetUse(uuid);
}

const std::string& AssetManager::GetTypeName(AssetType assetType) const {
	return assetTypeNames[static_cast<size_t>(assetType)];
}

void AssetManager::RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* importer) {
	assetTypeNames[static_cast<size_t>(assetType)] = typeName;
	assetTypeImporters[static_cast<size_t>(assetType)] = importer;
}
