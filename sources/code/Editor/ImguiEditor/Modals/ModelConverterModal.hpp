#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;

			class ModelConverterModal {
			public:
				ModelConverterModal();
				void show();
				void convertFile();
				void close();
				void render();
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
