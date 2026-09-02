#pragma once

#include <vector>
#include "PluginManifestData.hpp"

namespace Grindstone::Plugins {
	bool LoadPluginManifestLockFile(Grindstone::Plugins::EditorPluginManager* pluginManager, std::vector<ManifestData>& manifestData);
}
