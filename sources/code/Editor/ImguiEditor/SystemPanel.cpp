#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include "SystemPanel.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			SystemPanel::SystemPanel(ECS::SystemRegistrar* systemRegistrar) {
				this->systemRegistrar = systemRegistrar;
			}
			
			void SystemPanel::render() {
				if (ImGui::Begin("Systems", &isShowingPanel)) {

					for (auto& system : systemRegistrar->systemFactories) {
						renderSystem(system.first.c_str());
					}

					ImGui::End();
				}
			}

			void SystemPanel::renderSystem(const char *system) {
				ImGui::Text(system);
			}
		}
	}
}
