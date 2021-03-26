#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
		class SceneManager;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;

			class ImageConverterModal {
			public:
				ImageConverterModal();
				void show();
				void convertFile();
				void close();
				void render();
			private:
				bool isShown = false;
				bool isProcessing = false;
				std::string imagePath;
				std::string error;
			};
		}
	}
}
