#pragma once

#include <map>
#include <string>

#include <Common/Utilities/ModuleLoading.hpp>

#include "IPluginManager.hpp"
#include "Interface.hpp"

namespace Grindstone::Plugins {
	class Interface;

	class DefaultPluginManager : public Grindstone::Plugins::IPluginManager {
	public:
		enum class BinaryType {
			NoBuild,
			Cpp,
			Dotnet,
		};

		struct BinaryEntry {
			std::string binaryIdentifier;
			std::string binaryFilename;
			BinaryType buildType;
		};
		using LoadStageList = std::vector<BinaryEntry>;
		virtual ~DefaultPluginManager();
	
		virtual bool PreprocessPlugins() override;
		virtual void LoadPluginsByStage(std::string_view stageName) override;
		virtual void UnloadPluginsByStage(std::string_view stageName) override;
		virtual std::filesystem::path GetLibraryPath(std::string_view pluginName, std::string_view libraryName) override;

	protected:
		bool LoadModule(const std::string& path);
		void UnloadModule(const std::string& path);

		std::map<std::string, Utilities::Modules::Handle> pluginModules;
		std::map<std::string, LoadStageList> pluginsMapByLoadStage;
	};
}
