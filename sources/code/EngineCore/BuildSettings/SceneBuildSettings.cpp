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
			sceneName = Utils::Trim(fileContents.substr(start));
			if (!sceneName.empty()) {
				sceneUuids.push_back(sceneName);
			}

			break;
		}

		sceneName = Utils::Trim(fileContents.substr(start, end - start));
		sceneUuids.push_back(sceneName);
		start = end + 1;
	}
}

Grindstone::Uuid SceneBuildSettings::GetDefaultScene() const {
	if (sceneUuids.empty()) {
		return nullptr;
	}

	return sceneUuids[0];
}
