#pragma once

#include <entt/entt.hpp>

namespace Grindstone::Editor::ImguiEditor {
	class ImguiEditor;

	class ImageConverterModal {
	public:
		void Show();
		void Render();
		bool IsOpen() const;
	private:
		void Close();
		void ConvertFile();
		std::string GetInputPathWithProperSlashes();
		void RenderFormatCombo();
	private:
		bool shouldImportCubemap = false;
		bool isShown = false;
		bool isProcessing = false;
		char selectedFormatIndex = 0;
		std::string imagePath;
		std::string error;
	};
}
