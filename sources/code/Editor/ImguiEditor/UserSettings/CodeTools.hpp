#pragma once

#include <vector>
#include <string>
#include "../Settings/BaseSettingsPage.hpp"

namespace Grindstone::Editor::ImguiEditor::Settings {
	class CodeTools : public BasePage {
	public:
		virtual void Open() override;
		virtual void Render() override;
		virtual void Save() override;
		virtual void Reset() override;
	private:
		std::string msBuildPath;
	};
}
