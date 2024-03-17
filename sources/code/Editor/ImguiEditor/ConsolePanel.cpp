#include <imgui.h>
#include <entt/entt.hpp>
#include "Common/Event/PrintMessageEvent.hpp"
#include "Common/Event/EventType.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Editor/EditorManager.hpp"
#include "ConsolePanel.hpp"

#include "ImguiRenderer.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::ImguiEditor;

ConsolePanel::ConsolePanel(ImguiRenderer* imguiRenderer) {
	consoleErrorIcon = imguiRenderer->CreateTexture("consoleIcons/ConsoleError.png");
	consoleWarningIcon = imguiRenderer->CreateTexture("consoleIcons/ConsoleWarning.png");
	consoleTraceIcon = imguiRenderer->CreateTexture("consoleIcons/ConsoleTrace.png");
	consoleInfoIcon = imguiRenderer->CreateTexture("consoleIcons/ConsoleInfo.png");

	auto dispatcher = Editor::Manager::GetEngineCore().GetEventDispatcher();
	dispatcher->AddEventListener(Events::EventType::PrintMessage, std::bind(&ConsolePanel::AddConsoleMessage, this, std::placeholders::_1));
}

ImTextureID ConsolePanel::GetLogSeverityIcon(LogSeverity severity) {
	switch (severity) {
		default:
		case LogSeverity::Info:
			return consoleInfoIcon;
		case LogSeverity::Trace:
			return consoleTraceIcon;
		case LogSeverity::Warning:
			return consoleWarningIcon;
		case LogSeverity::Error:
			return consoleErrorIcon;
	}
}

void ConsolePanel::Render() {
	if (isShowingPanel) {
		ImGui::Begin("Console", &isShowingPanel);

		if (messageQueue.size() == 0) {
			ImGui::Text("No console messages found.");
		}

		if (ImGui::BeginTable("assetBrowserSplit", 2)) {
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, 24.0f);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			size_t i = 0;
			for (auto& msg : messageQueue) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(++i % 2 ? ImGuiCol_TableRowBgAlt : ImGuiCol_TableRowBg));
				ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);
				ImGui::TableNextColumn();
				auto icon = GetLogSeverityIcon(msg.logSeverity);
				ImGui::Image(icon, ImVec2(20.0f, 20.0f));
				ImGui::TableNextColumn();
				ImGui::TextWrapped(msg.message.c_str());
			}

			ImGui::EndTable();
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
