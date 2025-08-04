#pragma once

#include <vector>
#include "PluginManifestData.hpp"

namespace Grindstone::Plugins {
	bool LoadPluginManifestLockFile(std::vector<ManifestData>& manifestData);
}
