#pragma once

#include <vector>
#include <string>

namespace Grindstone {
	namespace BuildSettings {
		class PluginBuildSettings {
		public:
			PluginBuildSettings();
			void load();
			unsigned int getNumPlugins();
			const char* getPluginByNumber(unsigned int);
		private:
			std::vector<std::string> plugins;
		};
	}
}
