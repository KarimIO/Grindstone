#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;

			class ImageConverterModal {
			public:
				ImageConverterModal();
				void Show();
				void Render();
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
	}
}
