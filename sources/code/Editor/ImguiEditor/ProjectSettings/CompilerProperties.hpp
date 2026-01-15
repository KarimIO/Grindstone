#pragma once

#include <vector>
#include <string>
#include "../Settings/BaseSettingsPage.hpp"

namespace Grindstone::Editor::ImguiEditor::Settings {
	class CompilerProperties {
	public:
		void Open();
		void Render();
		void Save();
		void Reset();
	private:
		std::vector<std::string> preprocessorDefinitions;
	};
}
