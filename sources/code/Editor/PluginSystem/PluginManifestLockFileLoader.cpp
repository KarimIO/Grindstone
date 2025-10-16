#include <EngineCore/EngineCore.hpp>
#include <Editor/EditorManager.hpp>

#include "EditorPluginManager.hpp"
#include "PluginManifestLockFileLoader.hpp"

bool Grindstone::Plugins::LoadPluginManifestLockFile(std::vector<ManifestData>& manifestDataList) {
	auto pluginManager = static_cast<Grindstone::Plugins::EditorPluginManager*>(Grindstone::EngineCore::GetInstance().GetPluginManager());
	const std::vector<std::filesystem::path> &pluginsFolders = pluginManager->GetPluginsFolders();

	for (int i = manifestDataList.size() - 1; i >= 0; --i) {
		ManifestData& manifestData = manifestDataList[i];
		for (const std::filesystem::path& basePluginPath : pluginsFolders) {
			std::filesystem::path metaFilePath = basePluginPath / manifestData.pluginName;
			if (std::filesystem::exists(metaFilePath / "plugin.meta.json")) {
				manifestData.path = metaFilePath;
				break;
			}
		}

		if (manifestData.path.empty()) {
			GPRINT_ERROR_V(Grindstone::LogSource::Editor, "Couldn't find plugin '{}' after searching plugin folders.", manifestData.pluginName.c_str());
			manifestDataList.erase(manifestDataList.begin() + i);
		}
	}

	return true;
}
