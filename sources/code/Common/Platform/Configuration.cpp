#include "Configuration.hpp"

const char* Grindstone::Platform::ConfigurationToString(Grindstone::Platform::Configuration configuration) {
	switch (configuration) {
		case Grindstone::Platform::Configuration::Auto:
			return CMAKE_INTDIR;
		case Grindstone::Platform::Configuration::Debug:
			return "Debug";
		case Grindstone::Platform::Configuration::Release:
			return "Release";
		case Grindstone::Platform::Configuration::MinSizeRelease:
			return "MinSizeRel";
		case Grindstone::Platform::Configuration::ReleaseWithDebugInfo:
			return "RelWithDebInfo";
	}

	return "";
}
