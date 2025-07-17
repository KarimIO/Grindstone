#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include "ModelImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_MODEL_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter("fbx", ImportModel, modelImporterVersion);
			editorPluginInterface->RegisterAssetImporter("dae", ImportModel, modelImporterVersion);
			editorPluginInterface->RegisterAssetImporter("obj", ImportModel, modelImporterVersion);
		}
	}

	EDITOR_MODEL_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->DeregisterAssetImporter("fbx");
			editorPluginInterface->DeregisterAssetImporter("dae");
			editorPluginInterface->DeregisterAssetImporter("obj");
		}
	}
}
