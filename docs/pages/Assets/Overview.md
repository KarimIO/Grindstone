
# Grindstone Asset System Overview

The Grindstone Asset System provides a unified, extensible way to load, manage, and reference game content (assets) across both the editor and runtime. It supports reference counting, type-safe asset access, and optimized loading strategies through a dual loader system.

You can learn about creating new assets with the following guides:
 - [Custom Asset Runtime Integration Guide](CustomAssetRuntimeGuide.md)
 - [Custom Asset Editor Import Guide](CustomAssetEditorGuide.md)

---

## Purpose

The asset system abstracts asset management away from file IO, providing a consistent and efficient API for accessing resources like meshes, materials, textures, audio clips, and more.

It supports:
- **Hot-reloading** in the editor.
- **Reference-counted** memory management.
- **Type-safe access** using `AssetReference<T>`.
- **Batch-packed runtime loading** for performance.

---

## Core Components

### `Asset`
Base class for all asset types. Each asset:
- Has a UUID and name.
- Tracks a `referenceCount`.
- Has a `AssetLoadStatus` (e.g. `Ready`, `Loading`, `Missing`, etc.)

### `AssetReference<T>`
Type-safe smart reference to an asset. Handles:
- Automatic reference counting.
- Access to the underlying asset (with `Get()` and `GetUnchecked()`).
- Safe copying/moving of references.

> ⚠️ Asset data access is not guaranteed to be cheap — cache it if reused frequently.

### `AssetImporter`
An abstract interface for managing asset lifecycles. Implements:
- Load/unload logic.
- Reference counting.
- Reloading support.

Specialized via `SpecificAssetImporter<YourAssetStruct>` for each asset type.

### `AssetManager`
The global hub for managing all registered asset types. Responsibilities include:
- Mapping `AssetType` to importer.
- Queuing and executing asset reloads.
- Accessing assets by UUID or address.
- Registering new asset types.

---

## Extensibility

You can register new asset types in plugins using:

 - Runtime Assets:
    - `RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* assetImporter)`
    - `UnregisterAssetType(AssetType assetType)`
 - Editor Assets:
    - `RegisterAssetImporter(const char* extension, Grindstone::Editor::ImporterFactory importerFactory, Grindstone::Editor::ImporterVersion importerVersion)`
    - `DeregisterAssetImporter(const char* extension)`


---

## Loaders

Two kinds of loaders support different use cases:

### `FileAssetLoader`
Used in the **editor**.
- Loads assets directly from disk.
- Supports live editing, hot-reloading, and development workflows.

### `AssetPackLoader`
Used in **runtime builds**.
- Loads from optimized, consolidated files (asset packs).
- Uses a manifest for resolving UUIDs and metadata.
- Greatly improves performance and reduces file I/O.

---

## Supported Assets

Out of the box, Grindstone includes support for:
- `Mesh3dAsset`
- `MaterialAsset`
- `TextureAsset`
- `AudioClipAsset`
- `GraphicsPipelineSetAsset`
- `ComputePipelineSetAsset`

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
Assets::AssetManager* assetManager = engineCore->assetManager;
CustomAsset* customAssetReference = assetManager->GetAssetByUuid<CustomAsset>(uuid);
if (customAssetReference == nullptr) {
    return;
}

CustomAsset* customAsset = customAssetReference.Get();

// Use the asset...
```

#### Access by Address (Hashed String):

```cpp
CustomAsset* customAssetReference = assetManager->GetAssetReferenceByAddress<CustomAsset>("@Assets/test");
if (customAssetReference == nullptr) {
    return;
}

CustomAsset* customAsset = customAssetReference.Get();

// Use the asset...
```

---

## Notes

- UUIDs identify assets internally across the engine, which ensure unique identities for each asset.
- Address HashedStrings are human-friendly identifiers.
- Reference counts prevent premature unloading.
- Asset reloads are queued and applied during an update.
