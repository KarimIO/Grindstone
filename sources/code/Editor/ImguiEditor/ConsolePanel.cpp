#include <imgui.h>
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

			ImVec4 GetLogSeverityColor(LogSeverity severity) {
				switch (severity) {
					default:
					case LogSeverity::Info:
						return ImColor(1.f, 1.f, 1.f, 1.f);
					case LogSeverity::Trace:
						return ImColor(0.38f, 0.6f, 0.75f, 1.f);
					case LogSeverity::Warning:
						return ImColor(0.75f, 0.65f, 0.4f, 1.f);
					case LogSeverity::Error:
						return ImColor(0.6f, 0.3f, 0.3f, 1.f);
				}
			}

			void ConsolePanel::Render() {
				if (isShowingPanel) {
					ImGui::Begin("Console", &isShowingPanel);

					if (messageQueue.size() == 0) {
						ImGui::Text("No console messages found.");
					}

					for(auto &msg : messageQueue) {
						auto color = GetLogSeverityColor(msg.logSeverity);
						ImGui::PushStyleColor(ImGuiCol_Text, color);
						ImGui::Text("%s", msg.message.c_str());
						ImGui::PopStyleColor();
					}

					ImGui::End();
				}
			}

			bool ConsolePanel::AddConsoleMessage(Grindstone::Events::BaseEvent* ev) {
				auto printMsg = (Events::PrintMessageEvent*)ev;
				messageQueue.push_front(printMsg->message);

				while (messageQueue.size() > 100) {
					messageQueue.pop_back();
				}

				return true;
			}
		}
	}
}
