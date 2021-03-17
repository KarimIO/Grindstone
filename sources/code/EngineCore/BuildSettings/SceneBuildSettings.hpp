#pragma once

#include <vector>
#include <string>

namespace Grindstone {
	namespace BuildSettings {
		class SceneBuildSettings {
		public:
			SceneBuildSettings();
			void load();
			const char* getDefaultScene();
		private:
			std::vector<std::string> scenes;
		};
	}
}
