#include "AssetManager.hpp"

using namespace Grindstone;

AssetManager::AssetManager() {
	assetTypeNames[assetType] = "Undefined";
}

void AssetManager::LoadFile(AssetType assetType, Uuid& uuid) {
	return assetTypeImporters[assetType]->Load(uuid);
}

void AssetManager::LazyLoadFile(AssetType assetType, Uuid& uuid) {
	return assetTypeImporters[assetType]->LazyLoad(uuid);
}

const char* AssetManager::GetTypeName(AssetType assetType) {
	return assetTypeNames[assetType];
}
