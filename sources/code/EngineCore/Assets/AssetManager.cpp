#include "AssetManager.hpp"

// Assets
#include "Materials/MaterialAsset.hpp"
#include "Materials/MaterialImporter.hpp"
#include "Shaders/ShaderAsset.hpp"
#include "Shaders/ShaderImporter.hpp"
#include "Textures/TextureAsset.hpp"
#include "Shaders/ShaderAsset.hpp"

using namespace Grindstone;

AssetType ShaderAsset::assetType;
AssetType MaterialAsset::assetType;

AssetManager::AssetManager() {
	assetTypeNames.emplace_back("Undefined");
	assetTypeImporters.emplace_back(nullptr);

	RegisterAssetType<ShaderAsset, ShaderImporter>();
	RegisterAssetType<MaterialAsset, MaterialImporter>();
}

void AssetManager::LoadFile(AssetType assetType, Uuid uuid) {
	if (assetType < 1 || assetType >= assetTypeImporters.size()) {
		return;
	}

	return assetTypeImporters[assetType]->Load(uuid);
}

std::string& AssetManager::GetTypeName(AssetType assetType) {
	return assetTypeNames[assetType];
}
