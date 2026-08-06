Asset System {#Assets}
============

The %Grindstone Asset System provides a unified, extensible way to load, manage, and reference game content (assets) across both the editor and runtime. It supports reference counting, type-safe asset access, and optimized loading strategies through a dual loader system.

You can learn about creating new assets with the following guides:
 - @subpage CustomAssetRuntimeGuide
 - @subpage CustomAssetEditorGuide

---

## Purpose

The asset system abstracts asset management away from file IO, providing a consistent and efficient API for accessing resources like meshes, materials, textures, audio clips, and more.

It supports:
- **Hot-reloading** in the editor.
- **Reference-counted** memory management.
- **Type-safe access** using [AssetReference<T>](@ref Grindstone::AssetReference).
- **Batch-packed runtime loading** for performance.

---

## Core Components

### Asset
[Asset](@ref Grindstone::Asset) is a base class for all asset types. Each Asset:
- Has a [UUID](@ref Grindstone::Uuid) and name.
- Tracks a `referenceCount`.
- Has a [AssetLoadStatus](@ref Grindstone::AssetLoadStatus).

### AssetReference
[AssetReference<T>](@ref Grindstone::AssetReference) is a type-safe smart reference to an asset. Handles:
- Automatic reference counting.
- Access to the underlying asset (with `Get()` and `GetUnchecked()`).
- Safe copying/moving of references.

### AssetImporter
[AssetImporter](@ref Grindstone::AssetImporter) is an abstract interface for managing asset lifecycles. Implements:
- Load/unload logic.
- Reference counting.
- Reloading support.

It can be specialized via [SpecificAssetImporter](@ref Grindstone::SpecificAssetImporter)`<YourAssetStruct>` for each asset type.

### AssetManager
[AssetManager](@ref Grindstone::Assets::AssetManager) is the global hub for managing all registered asset types. Responsibilities include:
- Mapping [AssetType](@ref Grindstone::AssetType) to importer.
- Queuing and executing asset reloads.
- Accessing assets by [UUID](@ref Grindstone::Uuid) or address.
- Registering new asset types.

---

## Extensibility

You can register new asset types in plugins using these methods of a [PluginInterface](@ref Grindstone::Plugins::Interface):

 - Runtime Assets:
    - `RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter*)`
    - `UnregisterAssetType(AssetType assetType)`
 - Editor Assets:
    - `RegisterAssetImporter(const char* extension, Grindstone::Editor::ImporterFactory, Grindstone::Editor::ImporterVersion)`
    - `DeregisterAssetImporter(const char* extension)`


---

## Loaders

Two kinds of loaders support different use cases:

### FileAssetLoader
[FileAssetLoader](@ref Grindstone::Assets::FileAssetLoader) is used in the **editor**.
- Loads assets directly from disk.
- Supports live editing, hot-reloading, and development workflows.

### AssetPackLoader
[AssetPackLoader](@ref Grindstone::ArchiveAssetLoader) is used in **runtime builds**.
- Loads from optimized, consolidated files (asset packs).
- Uses a manifest for resolving [UUID](@ref Grindstone::Uuid)s and metadata.
- Greatly improves performance and reduces file I/O.

---

## Supported Assets

Out of the box, %%Grindstone includes support for:
- @ref Grindstone::Mesh3dAsset (See @ref Meshes)
- @ref Grindstone::MaterialAsset (See @ref Materials)
- @ref Grindstone::TextureAsset (See @ref Textures)
- @ref Grindstone::Audio::AudioClipAsset
- @ref PipelineSets
    - @ref Grindstone::GraphicsPipelineAsset
    - @ref Grindstone::ComputePipelineAsset

Each is implemented with its own importer and load logic.

---

## Usage Patterns

### Load an asset reference:


#### Access by AssetReference:

```cpp
Grindstone::AssetReference<CustomAsset>& customAssetReference = myComponent.customAsset;
CustomAsset* myAssetAsset = customAssetReference.Get();

if (myAssetAsset == nullptr) {
    return;
}

// Use the asset...
```

#### Access by UUID:

```cpp
Grindstone::Uuid uuid = /* Retrieved from the scene components. */;
Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;
CustomAsset* customAssetReference = assetManager->GetAssetByUuid<CustomAsset>(uuid);
if (customAssetReference == nullptr) {
    return;
}

CustomAsset* customAsset = customAssetReference.Get();

// Use the asset...
```

#### Access by Address (Hashed String):

```cpp
Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;
CustomAsset* customAssetReference = assetManager->GetAssetReferenceByAddress<CustomAsset>("@Assets/test");
if (customAssetReference == nullptr) {
    return;
}

CustomAsset* customAsset = customAssetReference.Get();

// Use the asset...
```

---

## Notes

- [UUID](@ref Grindstone::Uuid)s identify assets internally across the engine, which ensure unique identities for each asset.
- Address HashedStrings are human-friendly identifiers.
- Reference counts prevent premature unloading.
- Asset reloads are queued and applied during an update.
