#pragma once

#include <vector>
#include <string>
#include "../Settings/BaseSettingsPage.hpp"

namespace Grindstone::Editor::ImguiEditor::Settings{
	class Plugins : public BasePage {
	public:
		virtual void Open() override;
		virtual void Render() override;
	private:
		void WriteFile();
		std::vector<std::string> pluginList;
		bool hasPluginsChanged = false;
	};
}
