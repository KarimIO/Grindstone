#pragma once

#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/HashedString.hpp>
#include <Editor/Importers/ImporterManager.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>

struct ImGuiContext;

namespace Grindstone::Editor {
	class Manager;
}

namespace Grindstone::Plugins {
	class EditorPluginInterface : public BaseEditorInterface {
	public:
		virtual ImGuiContext * GetImguiContext() const;
		virtual Grindstone::Editor::Manager* GetEditorInstance() const;
		virtual void MapExtensionToImporterType(const char* extension, Grindstone::HashedString importerType);
		virtual void RegisterAssetImporter(
			Grindstone::HashedString importerType,
			Grindstone::Editor::ImporterData importerVersion
		);
		virtual void RegisterAssetTemplate(AssetType assetType, const char* name, const char* extension, const void* const sourcePtr, size_t sourceSize);
		virtual void UnmapExtensionToImporterType(const char* extension);
		virtual void DeregisterAssetImporter(Grindstone::HashedString importerType);
		virtual void DeregisterAssetTemplate(AssetType assetType);
	};
}
