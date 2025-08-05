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

		std::string name;
		std::string displayName;
		std::string version;
		std::string description;
		std::string author;
		std::vector<std::string> assets;
		std::vector<Dependency> dependencies;
		std::vector<Binary> binaries;
		std::filesystem::path cmakePath;
	};
}
