Registering Plugin Code {#RegisteringPluginCode}
============

Plugins may contain their own components, managers, systems, and implementations of various interfaces, as well as editor tools. But the engine will not know
about them unless the plugin announces them to the engine. This is done through a registration process.

---

## Plugin Implementation

Plugins are shared libraries (`.dll`, `.so`, etc.) that are found in the `bin` directory. Plugins have two functions that allow them to integrate with the engine:

- `InitializeModule(Grindstone::Plugins::PluginInterface*)`
- `ReleaseModule(Grindstone::Plugins::PluginInterface*)`

These functions allow the plugin to register and unregister important functionality with the engine. By convention, they are usually stored in an `EntryPoint.cpp` file.

---

## List of official Plugin Interface methods

This may not be up to date - it's best to check the official PluginInterface instead.

### Core Getters

These are used for getting core functional things from the `EngineCore`, to be used in setters for globals usually.

 - `Grindstone::EngineCore* GetEngineCore();`
 - `Grindstone::GraphicsAPI::Core* GetGraphicsCore();`
 - `Grindstone::Display GetMainDisplay();`
 - `Grindstone::HashedString::HashMap* GetHashedStringMap() const;`
 - `Grindstone::Logger::LoggerState* GetLoggerState() const;`
 - `Grindstone::Memory::AllocatorCore::AllocatorState* GetAllocatorState() const;`
 - `Grindstone::CvarSystem* GetCvarSystem() const;`
 - `Grindstone::Window* CreateDisplayWindow(Grindstone::Window::CreateInfo&);`
 - `uint8_t CountDisplays();`
 - `void RegisterGraphicsCore(Grindstone::GraphicsAPI::Core* core);`
 - `void RegisterWindowManager(Grindstone::WindowManager*);`
 - `void RegisterDisplayManager(Grindstone::DisplayManager*);`
 - `void EnumerateDisplays(Grindstone::Display* displays);`

### Base Registry functions

| Register Function               | Unregister Function              | Description                                                               |
|---------------------------------|----------------------------------|---------------------------------------------------------------------------|
| RegisterSystem                  | UnregisterSystem                 | [Systems](@ref Grindstone::ECS::SystemFactory) are registered in the [SystemRegistrar](@ref Grindstone::ECS::SystemRegistrar). They are functions that run every tick. |
| RegisterEditorSystem            | UnregisterEditorSystem           | [Editor Systems](@ref Grindstone::ECS::SystemFactory) are registered in the [SystemRegistrar](@ref Grindstone::ECS::SystemRegistrar) They are functions that run every tick in the editor only. |
| RegisterAssetRenderer           | UnregisterAssetRenderer          | [AssetRenderer](@ref Grindstone::BaseAssetRenderer) are registered in the [AssetRendererManager](@ref Grindstone::AssetRendererManager). They are used to render specific types of assets such as [Mesh3dRenderer](@ref Grindstone::Mesh3dRenderer) and [SkeletalMeshRenderer](@ref Grindstone::SkeletalMeshRenderer). |
| RegisterAssetType               | UnregisterAssetType              | [AssetType](@ref Grindstone::AssetType) are registered in the [AssetManager](@ref Grindstone::Assets::AssetManager). Check out @ref Assets for more information. |
| RegisterAssetType<T>            | UnregisterAssetType<T>           | Same as above but used as a template. For example, you can `RegisterAssetType<Grindstone::Mesh3dAsset>()`. |
| RegisterWorldContextFactory     | UnregisterWorldContextFactory    | [Systems](@ref Grindstone::WorldContext) are registered in the [WorldContextManager](@ref Grindstone::WorldContextManager) |
| RegisterWorldContextFactory<T>  | UnregisterWorldContextFactory<T> | Same as above but used as a template. For example, you can `RegisterWorldContextFactory<Grindstone::Physics::WorldContext>(physicsWorldContextName)` |
| RegisterComponent<T>            | UnregisterComponent<T>           | Components are registered in the [ComponentRegistrar](@ref Grindstone::ECS::ComponentRegistrar). Components are data that can be attached to entities, and can be referenced in [Systems](@ref Grindstone::ECS::SystemFactory). |

### Editor Registry functions

| Register Function               | Unregister Function              | Description                                                               |
|---------------------------------|----------------------------------|---------------------------------------------------------------------------|
| RegisterAssetImporter           | DeregisterAssetImporter         | [ImporterData](@ref Grindstone::Editor::ImporterData) take a source resource file, and outputs [assets](@ref CustomAssetEditorGuide) based on it. |
| MapExtensionToImporterType      | UnmapExtensionToImporterType     | Maps some `extension` (i.e.: `.wav`) to an AssetImporter, (i.e.: `AudioImporter`). |
| RegisterAssetTemplate           | DeregisterAssetTemplate          | Allows a user to create a default asset that can be created in the `AssetBrowser`. |
| RegisterThumbnailGenerator      | DeregisterThumbnailGenerator     | Registers an AssetThumbnailGenerator that takes the UUID of the asset, and generates a thumbnail image to be shown in the `AssetBrowser`. |
| RegisterMenuItem                | DeregisterMenuItem               | Links a menu item path (i.e.: `Edit/Baking/Lighting`) and a shortcut (i.e.: `Ctrl+Alt+L`) to a function to do an action. |
| RegisterProjectSettingsPage     | DeregisterProjectSettingsPage    | Registers a settings page with a display name and a factory function. |

---

## Example 1: Runtime Plugin

The main plugin interface provides a means of getting core systems to set as globals (such as the logger and allocator), and register custom components,
asset types, and systems among other things.

```cpp
#include "pch.hpp"

#include <entt/entt.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>

#include "Components/AudioListenerComponent.hpp"
#include "Components/AudioSourceComponent.hpp"
#include "AudioClipImporter.hpp"
#include "Core.hpp" // Core Audio System

Grindstone::Audio::Core* audioCore = nullptr;
Grindstone::Audio::AudioClipImporter* audioClipImporter = nullptr;

extern "C" {
	AUDIO_OPENAL_API void InitializeModule(Grindstone::Plugins::Interface* pluginInterface) {
    	// Setup static references
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

    	// Allocate the global shared objects for this library.
		audioCore = Grindstone::Memory::AllocatorCore::Allocate<Audio::Core>();
		audioClipImporter = AllocatorCore::Allocate<Audio::AudioClipImporter>();

    	// Register components, assets, and other logic.
		pluginInterface->RegisterComponent<AudioListenerComponent>();
		pluginInterface->RegisterComponent<AudioSourceComponent>(SetupAudioSourceComponent, DestroyAudioSourceComponent);
		pluginInterface->RegisterAssetType(AssetType::AudioClip, "AudioClip", audioClipImporter);
	}

	AUDIO_OPENAL_API void ReleaseModule(Plugins::Interface* pluginInterface) {
    Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->DeregisterAssetImporter("wav");
		}

    	// Unregister the assets and components.
		pluginInterface->UnregisterAssetType(AssetType::AudioClip);
		pluginInterface->UnregisterComponent<AudioSourceComponent>();
		pluginInterface->UnregisterComponent<AudioListenerComponent>();

    	// Free the global shared objects.
		AllocatorCore::Free(audioClipImporter);
		AllocatorCore::Free(audioCore);
	}
}
```

---

## Example 2: Editor Plugin

While custom tools (and games) may provide their own child classes of PluginInterface, Grindstone has an official [EditorPluginInterface](@ref Grindstone::Plugins::EditorPluginInterface). Here is an example of an editor-specific plugin, and how we can register an importer for an asset.

```cpp
#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include "AudioImporter.hpp"

extern "C" {
	EDITOR_AUDIO_IMPORTER_EXPORT void InitializeModule(Grindstone::Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());

		Grindstone::Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter("wav", Grindstone::Editor::Importers::ImportAudio, audioImporterVersion);
		}
	}

	EDITOR_AUDIO_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Grindstone::Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->DeregisterAssetImporter("wav");
		}
	}
}
```
