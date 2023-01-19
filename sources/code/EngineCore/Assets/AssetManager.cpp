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
		return nullptr;
	}

	AssetImporter* assetImporter = assetTypeImporters[assetType];

	void* loadedAsset = nullptr;
	if (assetImporter->TryGetIfLoaded(uuid, loadedAsset)) {
		return loadedAsset;
	}
	else {
		// TODO: Load file using the appropriate loader (file, asset pack, etc)
		// and pass the binary data to the importer.
		char* fileData = nullptr;
		size_t fileSize = 0;
		assetLoader->Load(uuid, fileData, fileSize);

		if (fileData == nullptr) {
			return nullptr;
		}

		return assetTypeImporters[assetType]->ProcessLoadedFile(uuid, fileData, fileSize);
	}
}

std::string& AssetManager::GetTypeName(AssetType assetType) {
	return assetTypeNames[assetType];
}
