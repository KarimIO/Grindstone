#pragma once

#include <vector>
#include "PluginManifestData.hpp"

namespace Grindstone::Plugins {
	bool LoadPluginManifestFile(std::vector<Grindstone::Plugins::ManifestData>& manifestData);
}
