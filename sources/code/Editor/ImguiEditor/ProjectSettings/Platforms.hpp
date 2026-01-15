#pragma once

#include <vector>
#include <string>
#include "../Settings/BaseSettingsPage.hpp"

namespace Grindstone::Editor::ImguiEditor::Settings {
	class Platforms : public BasePage {
	public:
		Platforms();
		virtual void Open() override;
		virtual void Render() override;
		virtual void Save() override;
		virtual void Reset() override;
	private:
		std::vector<Grindstone::UniquePtr<BasePage>> platformPages;
	};
}
