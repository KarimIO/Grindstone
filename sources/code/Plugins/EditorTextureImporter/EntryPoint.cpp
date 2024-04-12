#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include "TextureImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_TEXTURE_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter("hdr", ImportTexture);
			editorPluginInterface->RegisterAssetImporter("png", ImportTexture);
			editorPluginInterface->RegisterAssetImporter("tga", ImportTexture);
			editorPluginInterface->RegisterAssetImporter("bmp", ImportTexture);
			editorPluginInterface->RegisterAssetImporter("psd", ImportTexture);
			editorPluginInterface->RegisterAssetImporter("jpg", ImportTexture);
			editorPluginInterface->RegisterAssetImporter("jpeg", ImportTexture);
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
