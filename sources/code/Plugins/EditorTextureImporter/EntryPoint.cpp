#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include "TextureImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_TEXTURE_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter("hdr", ImportTexture, textureImporterVersion);
			editorPluginInterface->RegisterAssetImporter("png", ImportTexture, textureImporterVersion);
			editorPluginInterface->RegisterAssetImporter("tga", ImportTexture, textureImporterVersion);
			editorPluginInterface->RegisterAssetImporter("bmp", ImportTexture, textureImporterVersion);
			editorPluginInterface->RegisterAssetImporter("psd", ImportTexture, textureImporterVersion);
			editorPluginInterface->RegisterAssetImporter("jpg", ImportTexture, textureImporterVersion);
			editorPluginInterface->RegisterAssetImporter("jpeg", ImportTexture, textureImporterVersion);
		}
	}

	EDITOR_TEXTURE_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->DeregisterAssetImporter("hdr");
			editorPluginInterface->DeregisterAssetImporter("png");
			editorPluginInterface->DeregisterAssetImporter("tga");
			editorPluginInterface->DeregisterAssetImporter("bmp");
			editorPluginInterface->DeregisterAssetImporter("psd");
			editorPluginInterface->DeregisterAssetImporter("jpg");
			editorPluginInterface->DeregisterAssetImporter("jpeg");
		}
	}
}
