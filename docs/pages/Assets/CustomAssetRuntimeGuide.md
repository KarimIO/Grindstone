# Custom Asset Runtime Integration Guide

Although Grindstone supports many types of assets already, you may want to extend the system to support a new type of plugin or a custom format for your game.

> ⚠️ This is one of two tutorials related to asset loading. This is only about the runtime side of the asset - not importing the asset. You will not be able to use an asset unless you import the asset. Read more about that in the [Custom Asset Editor Import Guide](CustomAssetEditorGuide.md)

## Step 1: Create a Custom Asset Type

Define your asset struct, inheriting from `Asset`:

```cpp
// CustomAsset.hpp
#pragma once
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
    struct CustomAsset : public Asset {
        CustomAsset(Uuid uuid, const std::string& displayName) : Asset(uuid, displayName) {}
        
        int myVariable1;
        std::string myVariable2;

        DEFINE_ASSET_TYPE("My Custom Asset", AssetType::CustomAsset)
    };
}
```

---

## Step 2: Create an Importer for It

Your `CustomImporter` loads the relevant `.custom` file, parses headers, and initializes GPU resources.

```cpp
// CustomImporter.hpp
#pragma once
#include <map>
#include <EngineCore/Assets/AssetImporter.hpp>
#include "CustomAsset.hpp"

namespace Grindstone {
    class CustomImporter : public SpecificAssetImporter<CustomAsset, AssetType::CustomAsset> {
    public:
        virtual ~CustomImporter() override;
        virtual void* LoadAsset(Uuid uuid) override;
        virtual void QueueReloadAsset(Uuid uuid) override;

        bool ImportCustomFile(CustomAsset& mesh);
    };
}
```

```cpp
// CustomImporter.cpp
#include <Common/Logging.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include "CustomImporter.hpp"
using namespace Grindstone;

void CustomImporter::QueueReloadAsset(Uuid uuid) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();

	// Try find the file if it has already been loaded.
	auto customInMap = assets.find(uuid);
	if (customInMap == assets.end()) {
		return;
	}

	Grindstone::CustomAsset& asset = customInMap->second;
	asset.assetLoadStatus = AssetLoadStatus::Reloading;
	// Cleanup here if the asset was already loaded...

	// Load the file if it hasn't been loaded yet.
	ImportCustomFile(asset);
}

void* CustomImporter::LoadAsset(Uuid uuid) {
	auto& assetIterator = assets.emplace(uuid, CustomAsset(uuid, uuid.ToString()));
	CustomAsset& asset = assetIterator.first->second;

	asset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!ImportCustomFile(asset)) {
		return nullptr;
	}

	return &asset;
}

bool CustomImporter::ImportCustomFile(CustomAsset& asset) {
	Grindstone::Assets::AssetLoadBinaryResult result = engineCore->assetManager->LoadBinaryByUuid(AssetType::CustomAsset, asset.uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "CustomImporter::LoadAsset Unable to load file with id: {}", mesh.uuid.ToString());
		asset.assetLoadStatus = AssetLoadStatus::Missing;
		return false;
	}

	char* fileContent = reinterpret_cast<char*>(result.buffer.Get());
	uint64_t fileSize = result.buffer.GetCapacity();
	asset.name = result.displayName;

	// Import the asset here...
	asset.myVariable1 = 4;
	asset.myVariable2 = fileContent;

	asset.assetLoadStatus = AssetLoadStatus::Ready;
	return true;
}
```

---

## Step 3: Register the Asset Type

In your plugin `EntryPoint` file, register your new component and its importer:
 - `RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* assetImporter)`
 - `UnregisterAssetType(AssetType assetType)`

```cpp
// EntryPoint.cpp
#include "CustomImporter.hpp"

CustomImporter* customImporter = nullptr;

extern "C" {
    MY_PLUGIN_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
        // Setup important static
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Grindstone::EngineCore* engineCore = pluginInterface->GetEngineCore();
		EngineCore::SetInstance(*engineCore);

		customImporter = AllocatorCore::Allocate<CustomImporter>(engineCore);


        pluginInterface->RegisterAssetType(CustomAsset::GetStaticType(), "CustomAsset Display Name here", customImporter);
        }

    MY_PLUGIN_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		if (customImporter) {
			pluginInterface->UnregisterAssetType(CustomAsset::GetStaticType());
			AllocatorCore::Free(customImporter);
			customImporter = nullptr;
		}
    }
}
```

---

## Step 4: Load and Use the Asset

There are multiple ways to access the asset ingame.

### Access by AssetReference:

```cpp
struct CustomAssetComponent {
	Grindstone::AssetReference<CustomAsset> customAsset;

    REFLECT("CustomAsset")
};

Grindstone::AssetReference<CustomAsset>& customAssetReference = myComponent.customAsset;
CustomAsset* myAssetAsset = customAssetReference.Get();

if (myAssetAsset == nullptr) {
    return;
}

// Use the asset...
```

### Access by UUID:

```cpp
Grindstone::Uuid uuid = /* Retrieved from the scene components. */;
Assets::AssetManager* assetManager = engineCore->assetManager;
CustomAsset* customAssetReference = assetManager->GetAssetByUuid<CustomAsset>(uuid);
if (customAssetReference == nullptr) {
    return;
}

CustomAsset* customAsset = customAssetReference.Get();

// Use the asset...
```

### Access by Address (Hashed String):

```cpp
CustomAsset* customAssetReference = assetManager->GetAssetReferenceByAddress<CustomAsset>("@Assets/test");
if (customAssetReference == nullptr) {
    return;
}

CustomAsset* customAsset = customAssetReference.Get();

// Use the asset...
```

---
