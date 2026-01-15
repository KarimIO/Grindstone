#pragma once

namespace Grindstone::Editor::ImguiEditor::Settings {
	class BasePage {
	public:
		virtual void Open() = 0;
		virtual void Render() = 0;
		virtual void Save() = 0;
		virtual void Reset() = 0;
	};
}
