#pragma once

#include <set>
#include <map>
#include <string>
#include <vector>

#include <Common/Utilities/ModuleLoading.hpp>
#include <Editor/PluginSystem/PluginMetaData.hpp>
#include <Editor/PluginSystem/PluginManifestData.hpp>
#include <EngineCore/PluginSystem/IPluginManager.hpp>

#include "EditorPluginInterface.hpp"

namespace Grindstone::Plugins {
	class Interface;

	class EditorPluginManager : public IPluginManager {
	public:
		virtual ~EditorPluginManager();
			
		virtual bool PreprocessPlugins() override;
		virtual void LoadPluginsByStage(std::string_view stageName) override;
		virtual void UnloadPluginsByStage(std::string_view stageName) override;
		virtual std::filesystem::path GetLibraryPath(std::string_view pluginName, std::string_view libraryName) override;
		virtual void QueueInstall(std::string pluginName);
		virtual void QueueUninstall(std::string pluginName);
		virtual void ProcessQueuedPluginInstallsAndUninstalls();

		using Iterator = std::vector<Grindstone::Plugins::MetaData>::iterator;
		using ConstIterator = std::vector<Grindstone::Plugins::MetaData>::const_iterator;

		Iterator begin() noexcept;
		ConstIterator begin() const noexcept;

		Iterator end() noexcept;
		ConstIterator end() const noexcept;

	protected:
		bool LoadModule(const std::filesystem::path& path);
		void UnloadModule(const std::filesystem::path& path);
		void ResolvePlugins(std::vector<Grindstone::Plugins::ManifestData>& manifestResults);

		std::map<std::filesystem::path, Utilities::Modules::Handle> pluginModules;
		std::set<std::string> queuedInstalls{};
		std::set<std::string> queuedUninstalls{};
		std::vector<Grindstone::Plugins::MetaData> resolvedPluginManifest{};
	};
}
