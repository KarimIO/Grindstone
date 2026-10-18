#pragma once

#include <filesystem>
#include <vector>
#include <string>

namespace Grindstone::Plugins {
	struct MetaData {
		struct PluginDependency {
			std::string pluginName;
			std::string version;
		};

		enum class BinaryBuildType {
			NoBuild,
			Cmake,
			Dotnet,
		};

		struct Binary {
			std::string name;
			std::filesystem::path libraryRelativePath;
			std::string loadStage;
			BinaryBuildType buildType = BinaryBuildType::NoBuild;
			std::string buildTarget;
			std::vector<std::string> dependencies;
		};

		struct AssetDirectory {
			std::string name;
			std::filesystem::path assetDirectoryRelativePath;
			std::string mountPoint;
			std::string loadStage;
			std::vector<std::string> dependencies;
		};

		std::string name;
		std::string displayName;
		std::string version;
		std::string description;
		std::string author;
		std::filesystem::path pluginResolvedPath;
		bool isRestartRequired = false;
		std::vector<AssetDirectory> assetDirectories;
		std::vector<PluginDependency> requiresPlugins;
		std::vector<Binary> binaries;
		std::filesystem::path cmakePath;
	};
}
