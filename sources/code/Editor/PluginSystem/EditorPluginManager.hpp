#pragma once

#include <map>
#include <string>

#include <Common/Utilities/ModuleLoading.hpp>

#include <EngineCore/PluginSystem/IPluginManager.hpp>
#include "EditorPluginInterface.hpp"
#include "PluginMetaData.hpp"

namespace Grindstone::Plugins {
	class Interface;

	class EditorPluginManager : public IPluginManager {
	public:
		virtual ~EditorPluginManager();
			
		virtual bool PreprocessPlugins() override;
		virtual void LoadPluginsByStage(std::string_view stageName) override;
		virtual void UnloadPluginsByStage(std::string_view stageName) override;
		virtual std::filesystem::path GetLibraryPath(std::string_view pluginName, std::string_view libraryName) override;

	protected:
		bool LoadModule(const std::filesystem::path& path);
		void UnloadModule(const std::filesystem::path& path);

		std::map<std::filesystem::path, Utilities::Modules::Handle> pluginModules;
		std::vector<Grindstone::Plugins::MetaData> resolvedPluginManifest{};
	};
}
