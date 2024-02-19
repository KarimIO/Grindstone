#pragma once

#include <vector>
#include <string>

namespace Grindstone {
	namespace BuildSettings {
		class SceneBuildSettings {
		public:
			SceneBuildSettings();
			void Load();
			const char* GetDefaultScene() const;
		private:
			std::vector<std::string> scenes;
		};
	}
}
