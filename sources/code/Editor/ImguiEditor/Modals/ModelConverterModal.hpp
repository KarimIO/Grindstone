#pragma once

#include <entt/entt.hpp>

namespace Grindstone::Editor::ImguiEditor {
	class ImguiEditor;

	class ModelConverterModal {
	public:
		void Show();
		void ConvertFile();
		void Close();
		void Render();
		bool IsOpen() const;
	private:
		bool isShown = false;
		bool isProcessing = false;
		float modelScale = false;
		std::string modelPath;
		std::string error;
	};
}
