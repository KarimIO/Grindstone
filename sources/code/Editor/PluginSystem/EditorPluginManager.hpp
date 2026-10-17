#pragma once

#include <map>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <Common/Utilities/ModuleLoading.hpp>
#include <Editor/PluginSystem/PluginManifestData.hpp>
#include <Editor/PluginSystem/PluginMetaData.hpp>
#include <EngineCore/PluginSystem/IPluginManager.hpp>

namespace Grindstone::Plugins {
	class Interface;

	struct LoadItem {
		const Grindstone::Plugins::MetaData* plugin = nullptr;
		std::variant<const MetaData::Binary*, const MetaData::AssetDirectory*> loadable;
	};

	class EditorPluginManager : public IPluginManager {
	public:
		virtual ~EditorPluginManager();

		virtual bool PreprocessPlugins() override;
		virtual void LoadPluginsByStage(std::string_view stageName) override;
		virtual void UnloadPluginsByStage(std::string_view stageName) override;
		virtual std::filesystem::path GetLibraryPath(std::string_view pluginName, std::string_view libraryName) override;

		const std::vector<std::filesystem::path>& GetPluginsFolders();
		virtual void AddPluginsFolder(const std::filesystem::path& path);
		virtual void QueueInstall(const std::filesystem::path& path);
		virtual void QueueUninstall(const std::filesystem::path& path);
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
		bool BuildLoadOrder(std::span<const std::string_view> stageOrder);

		std::map<std::filesystem::path, Utilities::Modules::Handle> pluginModules;
		std::set<std::filesystem::path> queuedInstalls{};
		std::set<std::filesystem::path> queuedUninstalls{};
		std::vector<Grindstone::Plugins::MetaData> resolvedPluginManifest{};
		std::unordered_map<std::string, std::vector<LoadItem>> orderedItemsByStage;
		std::vector<std::filesystem::path> pluginsFolders{};
	};
}
