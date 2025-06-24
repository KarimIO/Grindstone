
# Grindstone Plugin System Roadmap

The Plugin system is core to the design of Grindstone. It allows games to be modular and extensible, without managing the overhead of unnecessary systems.

Plugins can be stored within a project, or downloaded from a network. When loaded, they should register themselves to expose their functionality to other plugins and the game code. When the plugin is unloaded, they must also clean up after themselves.

---

## Plugin List

The initial plugin system for Grindstone is very simple. Projects contain a simple text file `buildplugins.txt` that lists each plugin separated by a newline. Each row is simply the name of a shared library (`.dll`, `.so`, etc.) without the extension. The `PluginManager` will load each file in the same order as that of the list.

An example of a plugin file is:
```plaintext
PluginEditorAudioImporter
PluginEditorMaterialImporter
PluginEditorModelImporter
PluginEditorPipelineSetImporter
PluginEditorTextureImporter
PluginBulletPhysics
PluginRenderables3D
PluginRendererDeferred
```

---

## Plugin Implementation

Plugins are shared libraries (`.dll`, `.so`, etc.) that are found in the `bin` directory. Plugins have two functions that allow them to integrate with the engine:

- `InitializeModule(Grindstone::Plugins::PluginInterface*)`
- `ReleaseModule(Grindstone::Plugins::PluginInterface*)`

These functions allow the plugin to register and unregister important functionality with the engine. By convention, they are usually stored in an `EntryPoint.cpp` file.

### Example 1: Runtime Plugin

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

### Example 2: Editor Plugin


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

---

## Future Work

Thew upcoming plugin system design is described in [Plugin System 2.0 Specification](NewDesign.md)
