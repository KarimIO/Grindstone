#pragma once

#include <filesystem>
#include <vector>
#include <string>

namespace Grindstone::Plugins {
	struct MetaData {
		struct Dependency {
			std::string pluginName;
			std::string version;
		};

		struct Binary {
			std::filesystem::path libraryRelativePath;
			std::string loadStage;
		};

		struct AssetDirectory {
			std::filesystem::path assetDirectoryRelativePath;
			std::string mountPoint;
			std::string loadStage;
		};

		std::string name;
		std::string displayName;
		std::string version;
		std::string description;
		std::string author;
		std::vector<AssetDirectory> assetDirectories;
		std::vector<Dependency> dependencies;
		std::vector<Binary> binaries;
		std::filesystem::path cmakePath;
	};
}
