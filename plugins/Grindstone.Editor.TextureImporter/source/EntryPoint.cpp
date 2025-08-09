#include <Grindstone.Editor.TextureImporter/include/pch.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include <Grindstone.Editor.TextureImporter/include/TextureImporter.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Editor::Importers;

Grindstone::ConstHashedString textureImporterName("TextureImporter");

extern "C" {
	EDITOR_TEXTURE_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

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
		}
	}

	EDITOR_TEXTURE_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
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
