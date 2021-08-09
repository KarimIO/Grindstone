#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;

			class ModelConverterModal {
			public:
				ModelConverterModal();
				void Show();
				void ConvertFile();
				void Close();
				void Render();
			private:
				bool isShown = false;
				bool isProcessing = false;
				float modelScale = false;
				std::string modelPath;
				std::string error;
			};
		}
	}
}
