# Custom Asset Editor Import Guide

Although Grindstone supports many types of assets already, you may want to extend the system to support a new type of plugin or a custom format for your game.

> ⚠️ This is one of two tutorials related to asset loading. This is only about importing the source asset that the user may create. You will not be able to use the asset ingame unless you follow the other guide. Read more about that in the [Custom Asset Runtime Integration Guide](CustomAssetRuntimeGuide.md)

## Step 1: Create an asset importer function

This function will take the path of a file, and output any number of assets. Usually this involves parsing it, but the simplest importre may simply copy the file.

The list of assets will be included in a meta file, so for each asset, you must call one of the following functions, depending if the asset is the "main" asset, or another asset.
 - `GetOrCreateDefaultSubassetUuid(const std::string& subassetName, AssetType assetType)`
 - `GetOrCreateSubassetUuid(const std::string& subassetName, AssetType assetType)`

```cpp
#include <Common/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

void ImportCustomAsset(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& inputPath) {
	Grindstone::Editor::MetaFile* metaFile = assetRegistry.GetMetaFileByPath(path);

	Grindstone::Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, AssetType::CustomAsset);

	std::filesystem::path outputPath = assetRegistry.GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile->Save(materialImporterVersion);
	assetManager.QueueReloadAsset(AssetType::CustomAsset, uuid);
}
```

---

## Step 2: Register the Asset Importer

In your plugin `EntryPoint` file, register the asset importer and unregister on release.
 - `RegisterAssetImporter(const char* extension, Grindstone::Editor::ImporterFactory importerFactory, Grindstone::Editor::ImporterVersion importerVersion)`
 - `DeregisterAssetImporter(const char* extension)`

```cpp
// EntryPoint.cpp
#include "CustomImporter.hpp"

const Grindstone::Editor::ImporterVersion customImporterVersion = 1;

extern "C" {
	EDITOR_CUSTOM_IMPORTER_EXPORT void InitializeModule(Grindstone::Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Grindstone::Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter("customext", ImportCustomAsset, customImporterVersion);
		}
	}

	EDITOR_CUSTOM_IMPORTER_EXPORT void ReleaseModule(Grindstone::Plugins::Interface* pluginInterface) {
		Grindstone::Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->DeregisterAssetImporter("customext");
		}
	}
}
```
