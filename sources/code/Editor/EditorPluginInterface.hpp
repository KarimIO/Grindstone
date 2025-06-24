#pragma once

#include <Common/ResourcePipeline/AssetType.hpp>
#include <Editor/Importers/ImporterManager.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>

namespace Grindstone::Plugins {
	class EditorPluginInterface : public BaseEditorInterface {
	public:
		virtual void RegisterAssetImporter(
			const char* extension,
			Grindstone::Editor::ImporterFactory importerFactory,
			Grindstone::Editor::ImporterVersion importerVersion
		);
		virtual void RegisterAssetTemplate(AssetType assetType, const char* name, const char* extension, const void* const sourcePtr, size_t sourceSize);
		virtual void DeregisterAssetImporter(const char* extension);
		virtual void DeregisterAssetTemplate(AssetType assetType);
	};
}
