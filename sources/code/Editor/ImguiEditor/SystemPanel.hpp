#pragma once
namespace Grindstone {
	namespace ECS {
		class SystemRegistrar;
	}

	namespace Editor {
		namespace ImguiEditor {
			class SystemPanel {
			public:
				SystemPanel(ECS::SystemRegistrar* systemRegistrar);
				void render();
			private:
				void renderSystem(const char *system);
			private:
				bool isShowingPanel = true;
				ECS::SystemRegistrar* systemRegistrar;
			};
		}
	}
}
