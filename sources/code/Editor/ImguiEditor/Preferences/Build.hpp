#pragma once

#include <vector>
#include <string>
#include "BasePreferencesPage.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Preferences {
				class Build : public BasePage {
				public:
					virtual void Open() override;
					virtual void Render() override;
				private:
					void WriteFile();
					std::vector<std::string> sceneList;
				};
			}
		}
	}
}
