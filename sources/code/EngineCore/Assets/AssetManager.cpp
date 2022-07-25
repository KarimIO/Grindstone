#include "AssetManager.hpp"

using namespace Grindstone;

AssetManager::AssetManager() {
	assetTypeNames.emplace_back("Undefined");
	assetTypeImporters.emplace_back(nullptr);
}

void AssetManager::LoadFile(AssetType assetType, Uuid& uuid) {
	return assetTypeImporters[assetType]->Load(uuid);
}

void AssetManager::LazyLoadFile(AssetType assetType, Uuid& uuid) {
	return assetTypeImporters[assetType]->LazyLoad(uuid);
}

std::string& AssetManager::GetTypeName(AssetType assetType) {
	return assetTypeNames[assetType];
}
