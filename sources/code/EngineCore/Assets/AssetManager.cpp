#include "AssetManager.hpp"
#include "Loaders/AssetLoader.hpp"
#include "Loaders/FileAssetLoader.hpp"

// Assets
#include "Materials/MaterialAsset.hpp"
#include "Materials/MaterialImporter.hpp"
#include "Shaders/ShaderAsset.hpp"
#include "Shaders/ShaderImporter.hpp"
#include "Textures/TextureAsset.hpp"
#include "Shaders/ShaderAsset.hpp"

using namespace Grindstone;
using namespace Grindstone::Assets;

AssetType ShaderAsset::assetType;
AssetType MaterialAsset::assetType;

AssetManager::AssetManager() {
	assetLoader = new FileAssetLoader();
	assetTypeNames.emplace_back("Undefined");
	assetTypeImporters.emplace_back(nullptr);

	RegisterAssetType<ShaderAsset, ShaderImporter>();
	RegisterAssetType<MaterialAsset, MaterialImporter>();
}

void* AssetManager::GetAsset(AssetType assetType, Uuid uuid) {
	if (assetType < 1 || assetType >= assetTypeImporters.size()) {
		return;
	}

	// TODO: Load file using the appropriate loader (file, asset pack, etc)
	// and pass the binary data to the importer.
	AssetImporter* assetImporter = assetTypeImporters[assetType];

	void* loadedAsset = nullptr;
	if (assetImporter->TryGetIfLoaded(uuid, loadedAsset)) {
		return loadedAsset;
	}
	else {
		std::vector<char> fileContents;
		assetLoader->Load(uuid, fileContents);
		return assetTypeImporters[assetType]->Load(uuid);
	}
}

std::string& AssetManager::GetTypeName(AssetType assetType) {
	return assetTypeNames[assetType];
}
