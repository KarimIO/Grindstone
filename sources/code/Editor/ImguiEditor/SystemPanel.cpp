#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include "SystemPanel.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			SystemPanel::SystemPanel(ECS::SystemRegistrar* systemRegistrar) {
				this->systemRegistrar = systemRegistrar;
			}
			
			void SystemPanel::Render() {
				if (isShowingPanel) {
					ImGui::Begin("Systems", &isShowingPanel);

					for (auto& system : systemRegistrar->systemFactories) {
						RenderSystem(system.first.c_str());
					}

					ImGui::End();
				}
			}

			void SystemPanel::RenderSystem(const char *system) {
				ImGui::Text(system);
			}
		}
	}
}
