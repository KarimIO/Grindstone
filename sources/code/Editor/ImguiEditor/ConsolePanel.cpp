#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include "Common/Event/PrintMessageEvent.hpp"
#include "Common/Event/EventType.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Editor/EditorManager.hpp"
#include "ConsolePanel.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			ConsolePanel::ConsolePanel() {
				auto dispatcher = Editor::Manager::GetEngineCore().GetEventDispatcher();
				dispatcher->AddEventListener(Events::EventType::PrintMessage, std::bind(&ConsolePanel::AddConsoleMessage, this, std::placeholders::_1));
			}
			
			void ConsolePanel::Render() {
				if (isShowingPanel) {
					ImGui::Begin("Console", &isShowingPanel);

					if (messageQueue.size() == 0) {
						ImGui::Text("No console messages found.");
					}

					for(auto &msg : messageQueue) {
						ImGui::Text("%s", msg.message.c_str());
					}

					ImGui::End();
				}
			}

			bool ConsolePanel::AddConsoleMessage(Grindstone::Events::BaseEvent* ev) {
				auto printMsg = (Events::PrintMessageEvent*)ev;
				messageQueue.push_back(printMsg->message);

				while (messageQueue.size() > 5) {
					messageQueue.pop_front();
				}

				return true;
			}
		}
	}
}
