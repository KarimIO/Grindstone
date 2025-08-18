#include "PluginManifestLockFileLoader.hpp"

bool Grindstone::Plugins::LoadPluginManifestLockFile(std::vector<ManifestData>& manifestDataList) {
	std::filesystem::path basePluginPath = "plugins";
	for (ManifestData& manifestData : manifestDataList) {
		manifestData.path = basePluginPath / manifestData.pluginName;
	}

	return true;
}
