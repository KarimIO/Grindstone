#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;
	
	namespace Editor {
		class ComponentInspector;

		namespace ImguiEditor {
			class InspectorPanel {
			public:
				InspectorPanel(EngineCore* engineCore);
				void render();
			private:
				bool isShowingPanel = true;
				EngineCore* engineCore = nullptr;
				ComponentInspector* componentInspector = nullptr;
			};
		}
	}
}
