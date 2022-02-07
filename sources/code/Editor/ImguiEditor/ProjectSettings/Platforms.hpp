#pragma once

#include <vector>
#include <string>
#include "../Settings/BaseSettingsPage.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class Platforms : public BasePage {
				public:
					Platforms();
					virtual void Open() override;
					virtual void Render() override;
				private:
					void WriteFile();
					std::vector<BasePage*> platformPages;
				};
			}
		}
	}
}
