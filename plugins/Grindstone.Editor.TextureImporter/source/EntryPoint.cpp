#include <Grindstone.Editor.TextureImporter/include/pch.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Editor/PluginSystem/EditorPluginInterface.hpp>

#include <Grindstone.Editor.TextureImporter/include/TextureImporter.hpp>
#include <Grindstone.Editor.TextureImporter/include/ThumbnailGenerator.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Editor::Importers;

Grindstone::ConstHashedString textureImporterName("TextureImporter");

extern "C" {
	EDITOR_TEXTURE_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		Grindstone::EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter(textureImporterName, Grindstone::Editor::ImporterData{
				.importerVersion = textureImporterVersion,
				.factory = ImportTexture
			});
			editorPluginInterface->MapExtensionToImporterType("png", textureImporterName);
			editorPluginInterface->MapExtensionToImporterType("tga", textureImporterName);
			editorPluginInterface->MapExtensionToImporterType("bmp", textureImporterName);
			editorPluginInterface->MapExtensionToImporterType("psd", textureImporterName);
			editorPluginInterface->MapExtensionToImporterType("jpg", textureImporterName);
			editorPluginInterface->MapExtensionToImporterType("jpeg", textureImporterName);
			if (InitializeTextureThumbnailGenerator()) {
				editorPluginInterface->RegisterThumbnailGenerator(Grindstone::AssetType::Texture, Grindstone::Editor::Importers::GenerateTextureThumbnail);
			}
		}
	}

	EDITOR_TEXTURE_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->DeregisterThumbnailGenerator(Grindstone::AssetType::Texture, Grindstone::Editor::Importers::GenerateTextureThumbnail);
			Grindstone::Editor::Importers::ReleaseTextureThumbnailGenerator();
			editorPluginInterface->UnmapExtensionToImporterType("png");
			editorPluginInterface->UnmapExtensionToImporterType("tga");
			editorPluginInterface->UnmapExtensionToImporterType("bmp");
			editorPluginInterface->UnmapExtensionToImporterType("psd");
			editorPluginInterface->UnmapExtensionToImporterType("jpg");
			editorPluginInterface->UnmapExtensionToImporterType("jpeg");
			editorPluginInterface->DeregisterAssetImporter(textureImporterName);
		}
	}
}
