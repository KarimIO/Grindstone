#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "TextureImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_TEXTURE_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorRegisterAssetImporter("hdr", ImportTexture);
		pluginInterface->EditorRegisterAssetImporter("png", ImportTexture);
		pluginInterface->EditorRegisterAssetImporter("tga", ImportTexture);
		pluginInterface->EditorRegisterAssetImporter("bmp", ImportTexture);
		pluginInterface->EditorRegisterAssetImporter("psd", ImportTexture);
		pluginInterface->EditorRegisterAssetImporter("jpg", ImportTexture);
		pluginInterface->EditorRegisterAssetImporter("jpeg", ImportTexture);
	}

	EDITOR_TEXTURE_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorDeregisterAssetImporter("hdr");
		pluginInterface->EditorDeregisterAssetImporter("png");
		pluginInterface->EditorDeregisterAssetImporter("tga");
		pluginInterface->EditorDeregisterAssetImporter("bmp");
		pluginInterface->EditorDeregisterAssetImporter("psd");
		pluginInterface->EditorDeregisterAssetImporter("jpg");
		pluginInterface->EditorDeregisterAssetImporter("jpeg");
	}
}
