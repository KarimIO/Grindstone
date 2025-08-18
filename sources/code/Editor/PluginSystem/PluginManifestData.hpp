#pragma once

#include <string>
#include <filesystem>

namespace Grindstone::Plugins {
	struct ManifestData {
		std::string pluginName;
		std::string semanticVersioning;
		std::filesystem::path path;
	};
}
