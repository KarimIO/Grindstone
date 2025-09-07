#pragma once

#include <vector>
#include <string>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include "BaseSettingsPage.hpp"

namespace Grindstone::Editor::ImguiEditor::Settings {
	struct PageData {
		std::string title;
		Grindstone::UniquePtr<BasePage> page = nullptr;
	};

	class SettingsWindow {
	public:
		virtual ~SettingsWindow() {};

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
