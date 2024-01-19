#pragma once

#include <vector>
#include <string>

namespace Grindstone::Editor::ImguiEditor::Settings {
	class BasePage;

	struct PageData {
		std::string title;
		BasePage* page = nullptr;
	};

	class SettingsWindow {
	public:
		~SettingsWindow();

		void Open();
		void OpenPage(size_t preferencesPage);
		void Render();
		void RenderSettingsPage();
		void RenderSideBar();
		bool IsOpen() const;
	protected:
		bool isOpen = false;
		size_t settingIndex = 0;
		std::string settingsTitle;
		std::vector<PageData> pages;
	};
}
