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

// Loads an actual file, not an asset. The file still needs to be imported and handled.
// Make loading indirect to conceal how files are loading from asset importers
// This allows multiple files to be imported per asset, and allows different types of loading depending on the
// asset, such as shaders loading from a file if it's not compiled yet, or loading a compiled shader if it is loaded
bool Grindstone::Assets::AssetManager::LoadFile(Uuid uuid, char*& fileData, size_t& fileSize) {
	assetLoader->Load(uuid, fileData, fileSize);
	return fileData != nullptr;
}

std::string& AssetManager::GetTypeName(AssetType assetType) {
	return assetTypeNames[static_cast<size_t>(assetType)];
}

void AssetManager::RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* importer) {
	assetTypeNames[static_cast<size_t>(assetType)] = typeName;
	assetTypeImporters[static_cast<size_t>(assetType)] = importer;
}
