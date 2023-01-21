#include "SceneBuildSettings.hpp"
using namespace Grindstone::BuildSettings;

SceneBuildSettings::SceneBuildSettings() {
	Load();
}

void SceneBuildSettings::Load() {
	const char *path = "../BuildSettings/BuildSettings.json";

	scenes.push_back("scenes/test.scene.json");
}

const char* SceneBuildSettings::GetDefaultScene() {
	if (scenes.size() == 0) {
		return nullptr;
	}
	
	return scenes[0].c_str();
}
