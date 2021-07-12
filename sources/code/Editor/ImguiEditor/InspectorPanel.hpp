#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;
	
	namespace Editor {

		namespace ImguiEditor {
			class ComponentInspector;
			class MaterialInspector;

			class InspectorPanel {
			public:
				InspectorPanel(EngineCore* engineCore);
				void deselect();
				void selectFile(std::string selectedFileType, std::string selectedFilePath);
				void selectEntity(entt::entity selectedEntity);
				void render();
			private:
				std::string selectedFileType;
				std::string selectedFilePath;
				entt::entity selectedEntity = entt::null;
				bool isShowingPanel = true;
				EngineCore* engineCore = nullptr;
				ComponentInspector* componentInspector = nullptr;
				MaterialInspector* materialInspector = nullptr;
			};
		}
	}
}
