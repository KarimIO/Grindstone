#include "SceneBuildSettings.hpp"
using namespace Grindstone::BuildSettings;

SceneBuildSettings::SceneBuildSettings() {
	load();
}

void SceneBuildSettings::load() {
	const char *path = "../BuildSettings/BuildSettings.json";

	scenes.push_back("../assets/scenes/test.scene.json");
}

const char* SceneBuildSettings::getDefaultScene() {
	if (scenes.size() == 0) {
		return nullptr;
	}
	
	return scenes[0].c_str();
}
