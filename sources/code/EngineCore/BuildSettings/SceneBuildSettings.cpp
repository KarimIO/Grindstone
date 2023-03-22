#include "SceneBuildSettings.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
using namespace Grindstone::BuildSettings;

SceneBuildSettings::SceneBuildSettings() {
	Load();
}

void SceneBuildSettings::Load() {
	auto& engineCore = EngineCore::GetInstance();
	std::filesystem::path pluginListFile = engineCore.GetProjectPath() / "buildSettings/scenesManifest.txt";
	auto prefabListFilePath = pluginListFile.string();
	auto fileContents = Utils::LoadFileText(prefabListFilePath.c_str());

	size_t start = 0, end;
	std::string sceneName;
	while (true) {
		end = fileContents.find("\n", start);
		if (end == std::string::npos) {
			sceneName = fileContents.substr(start);
			if (!sceneName.empty()) {
				scenes.push_back(sceneName);
			}

			break;
		}

		sceneName = fileContents.substr(start, end - start);
		start = end + 1;
	}
}

const char* SceneBuildSettings::GetDefaultScene() {
	if (scenes.size() == 0) {
		return nullptr;
	}
	
	return scenes[0].c_str();
}
