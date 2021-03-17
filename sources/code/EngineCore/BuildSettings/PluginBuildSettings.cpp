#include <stdexcept>
	
#include "PluginBuildSettings.hpp"
using namespace Grindstone::BuildSettings;

PluginBuildSettings::PluginBuildSettings() {
	load();
}

void PluginBuildSettings::load() {
	const char *path = "../BuildSettings/Plugins.json";

	plugins.push_back("assets/scenes/sponza.json");
}

unsigned int PluginBuildSettings::getNumPlugins() {
	return (unsigned int)plugins.size();
}

const char* PluginBuildSettings::getPluginByNumber(unsigned int i) {
	if (i >= plugins.size()) {
		throw std::runtime_error("Invalid plugin id.");
	}

	return plugins[i].c_str();
}
