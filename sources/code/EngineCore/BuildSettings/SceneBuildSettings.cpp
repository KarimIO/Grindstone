#include "SceneBuildSettings.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
using namespace Grindstone::BuildSettings;

SceneBuildSettings::SceneBuildSettings() {
	Load();
}

void SceneBuildSettings::Load() {
	const EngineCore& engineCore = EngineCore::GetInstance();
	const std::filesystem::path sceneListFilePath = engineCore.GetProjectPath() / "buildSettings/scenesManifest.txt";
	const std::string fileContents = Utils::LoadFileText(sceneListFilePath.string().c_str());

	size_t start = 0;
	std::string sceneName;
	while (true) {
		const size_t end = fileContents.find('\n', start);
		if (end == std::string::npos) {
			sceneName = fileContents.substr(start);
			if (!sceneName.empty()) {
				scenes.push_back(sceneName);
			}

			break;
		}

		sceneName = fileContents.substr(start, end - start);
		scenes.push_back(sceneName);
		start = end + 1;
	}
}
const char* SceneBuildSettings::GetDefaultScene() const {
	if (scenes.empty()) {
		return nullptr;
	}

	return scenes[0].c_str();
}
