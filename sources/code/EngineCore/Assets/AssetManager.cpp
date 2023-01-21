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
		return assetTypeImporters[assetType]->ProcessLoadedFile(uuid);
	}
}

// Loads an actual file, not an asset. The file still needs to be imported and handled.
// Make loading indirect to conceal how files are loading from asset importers
// This allows multiple files to be imported per asset, and allows different types of loading depending on the
// asset, such as shaders loading from a file if it's not compiled yet, or loading a compiled shader if it is loaded
bool Grindstone::Assets::AssetManager::LoadFile(Uuid uuid, char*& fileData, size_t& fileSize) {
	assetLoader->Load(uuid, fileData, fileSize);
	return fileData != nullptr;
}

std::string& AssetManager::GetTypeName(AssetType assetType) {
	return assetTypeNames[assetType];
}
