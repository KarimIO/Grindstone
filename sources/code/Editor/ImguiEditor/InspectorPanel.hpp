#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;
	
	namespace Editor {

		namespace ImguiEditor {
			class ComponentInspector;
			class MaterialInspector;
			class v;

			class InspectorPanel {
			public:
				InspectorPanel(EngineCore* engineCore, ImguiEditor* imguiEditor);
				void Render();
			private:
				void RenderContents();
				bool isShowingPanel = true;
				EngineCore* engineCore = nullptr;
				ComponentInspector* componentInspector = nullptr;
				MaterialInspector* materialInspector = nullptr;
			};
		}
	}
}
