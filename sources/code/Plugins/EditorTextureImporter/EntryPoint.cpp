#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "TextureImporter.hpp"

using namespace Grindstone;

extern "C" {
	EDITOR_TEXTURE_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		AddImporterFactory("hdr", ImportTexture);
		AddImporterFactory("jpeg", ImportTexture);
		AddImporterFactory("jpg", ImportTexture);
		AddImporterFactory("png", ImportTexture);
		AddImporterFactory("tga", ImportTexture);
		AddImporterFactory("bmp", ImportTexture);
		AddImporterFactory("psd", ImportTexture);
	}

	EDITOR_TEXTURE_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
