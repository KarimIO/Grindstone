#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;

			class ImageConverterModal {
			public:
				ImageConverterModal();
				void show();
				void render();
			private:
				void close();
				void convertFile();
				std::string getInputPathWithProperSlashes();
				void renderFormatCombo();
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
