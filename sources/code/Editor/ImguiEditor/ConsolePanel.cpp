#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>

#include "Common/Event/PrintMessageEvent.hpp"
#include "Common/Event/EventType.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Editor/EditorManager.hpp"
#include "ImguiRenderer.hpp"

#include "ConsolePanel.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::ImguiEditor;

constexpr uint8_t fatalBit = (1 << static_cast<uint8_t>(LogSeverity::Fatal));
constexpr uint8_t errorBit = (1 << static_cast<uint8_t>(LogSeverity::Error));
constexpr uint8_t warnBit = (1 << static_cast<uint8_t>(LogSeverity::Warning));
constexpr uint8_t infoBit = (1 << static_cast<uint8_t>(LogSeverity::Info));
constexpr uint8_t traceBit = (1 << static_cast<uint8_t>(LogSeverity::Trace));

ConsolePanel::ConsolePanel(ImguiRenderer* imguiRenderer) : severityFlags(0xff), sourceFlags(UINT64_MAX) {
	consoleErrorIcon = imguiRenderer->CreateTexture("consoleIcons/ConsoleError.dds");
	consoleWarningIcon = imguiRenderer->CreateTexture("consoleIcons/ConsoleWarning.dds");
	consoleTraceIcon = imguiRenderer->CreateTexture("consoleIcons/ConsoleTrace.dds");
	consoleInfoIcon = imguiRenderer->CreateTexture("consoleIcons/ConsoleInfo.dds");

	auto dispatcher = Editor::Manager::GetEngineCore().GetEventDispatcher();
	dispatcher->AddEventListener(Events::EventType::PrintMessage, std::bind(&ConsolePanel::AddConsoleMessage, this, std::placeholders::_1));
}

void ConsolePanel::FilterSearch() {
	filteredMessage.clear();

	for (auto& message : messageQueue) {
		uint8_t severityBit = (1u << static_cast<uint8_t>(message.base.severity));
		uint64_t sourceBit = (1ull << static_cast<uint64_t>(message.base.source));
		bool showSeverity = (severityFlags & severityBit) > 0;
		bool showSource = (sourceFlags & sourceBit) > 0;
		if (
			showSeverity && showSource &&
			message.lowercaseMessage.find(filterTextLowercase) != std::string::npos
		) {
			filteredMessage.push_back(&message);
		}
	}
}

void ConsolePanel::RenderButton(const char* title, ImTextureID icon, uint8_t severityBit) {
	constexpr ImVec4 deselectedColor = ImVec4(1.f, 1.f, 1.f, 0.1f);
	constexpr ImVec4 deselectedHighlightColor = ImVec4(1.f, 1.f, 1.f, 0.2f);
	constexpr ImVec4 deselectedActiveColor = ImVec4(1.f, 1.f, 1.f, 0.3f);

	constexpr float btnSize = 18;

	bool isSelected = severityBit & severityFlags;

	ImGui::PushStyleColor(ImGuiCol_Button, isSelected ? selectedColor : deselectedColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, isSelected ? selectedActiveColor : deselectedActiveColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, isSelected ? selectedHighlightColor : deselectedHighlightColor);

	if (ImGui::ImageButton(title, icon, ImVec2(btnSize, btnSize))) {
		severityFlags = severityBit ^ severityFlags;
		FilterSearch();
	}
	ImGui::PopStyleColor(3);
}

void ConsolePanel::RenderTopbar() {
	ImGuiID consoleTopBar = ImGui::GetID("#consoleTopBar");
	ImGui::BeginChildFrame(consoleTopBar, ImVec2(0, 32), ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

	selectedColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
	selectedHighlightColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
	selectedActiveColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

	RenderButton("ConsoleErrorToggle", consoleErrorIcon, errorBit);
	ImGui::SameLine();
	RenderButton("ConsoleWarnToggle", consoleWarningIcon, warnBit);
	ImGui::SameLine();
	RenderButton("ConsoleInfoToggle", consoleInfoIcon, infoBit);
	ImGui::SameLine();
	RenderButton("ConsoleTraceToggle", consoleTraceIcon, traceBit);
	ImGui::SameLine();

	bool areAllSelected = sourceFlags == UINT64_MAX;
	uint64_t none = UINT64_MAX << (static_cast<uint64_t>(Grindstone::LogSource::Count));
	bool areNoneSelected = sourceFlags == none;

	std::string label;
	if (areAllSelected) {
		label = "Log Source Filter: All";
	}
	else if (areNoneSelected) {
		label = "Log Source Filter: None";
	}
	else {
		uint32_t selected = 0;
		uint32_t numSelected = 0;
		for (uint64_t i = 0; i < static_cast<uint64_t>(Grindstone::LogSource::Count); ++i) {
			if (sourceFlags & (1ull << i)) {
				numSelected += 1u;
				selected = i;

				if (numSelected == 2) {
					break;
				}
			}
		}

		label = "Log Source Filter: ";
		label += numSelected > 1
			? "Mixed"
			: logSourceStrings[selected];
	}

	ImGui::PushItemWidth(200.0f);
	if (ImGui::BeginCombo("##ConsoleSourceFilter", label.c_str())) {
		if (ImGui::Selectable("[ All ]", &areAllSelected)) {
			sourceFlags = UINT64_MAX;
			FilterSearch();
		}
		if (ImGui::Selectable("[ None ]", &areNoneSelected)) {
			sourceFlags = none;
			FilterSearch();
		}
		for (uint64_t i = 0; i < static_cast<uint64_t>(Grindstone::LogSource::Count); ++i) {
			bool bitEnabled = sourceFlags & (static_cast<uint64_t>(1u) << i);
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 64.0f);
			if (ImGui::Selectable(Grindstone::logSourceStrings[i], bitEnabled)) {
				sourceFlags ^= (static_cast<uint64_t>(1u) << i);
				FilterSearch();
			}
			ImGui::PopStyleVar();
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	float totalAvailWidth = ImGui::GetContentRegionAvail().x;
	const float searchWidth = 160.0f;
	const float searchX = totalAvailWidth - searchWidth;

	ImGui::SameLine(totalAvailWidth - 160.0f);
	ImGui::PushItemWidth(searchWidth);
	if (ImGui::InputTextWithHint("##Search", "Search", &filterText)) {
		if (filterText.empty()) {
			filterTextLowercase = "";
			FilterSearch();
		}
		else {
			filterTextLowercase.resize(filterText.size());
			std::transform(
				filterText.begin(),
				filterText.end(),
				filterTextLowercase.begin(),
				::tolower
			);
			FilterSearch();
		}
	}
	ImGui::EndChildFrame();
}

ImTextureID ConsolePanel::GetLogSeverityIcon(LogSeverity severity) const {
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

void ConsolePanel::RenderMessage(size_t index, EditorConsoleMessage& msg) {
	ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(index % 2 ? ImGuiCol_TableRowBgAlt : ImGuiCol_TableRowBg));
	ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);
	ImGui::TableNextColumn();
	auto icon = GetLogSeverityIcon(msg.base.severity);
	ImGui::Image(icon, ImVec2(20.0f, 20.0f));
	ImGui::TableNextColumn();
	ImGui::TextWrapped(msg.base.message.c_str());
	if (ImGui::IsItemHovered()) {
		time_t coarse = std::chrono::system_clock::to_time_t(msg.base.timepoint);
		std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> fine =
			std::chrono::time_point_cast<std::chrono::milliseconds>(msg.base.timepoint);
		const unsigned long long milliseconds = fine.time_since_epoch().count() % 1000u;

		tm tm;
		localtime_s(&tm, &coarse);

		char timeBuffer[sizeof("23:59:59.999")]{};
		size_t timeOffset = std::strftime(timeBuffer, sizeof timeBuffer - 3, "%T.", &tm);
		std::snprintf(
			timeBuffer + timeOffset,
			4, "%03llu",
			milliseconds
		);

		ImGui::SetTooltip("%s:%u [%s]", msg.base.filename.c_str(), msg.base.line, timeBuffer);
	}
}

void ConsolePanel::Render() {
	if (isShowingPanel) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
		ImGui::Begin("Console", &isShowingPanel);
		ImGui::PopStyleVar();
		float height = ImGui::GetContentRegionAvail().y - 36.0f;

		RenderTopbar();

		ImGuiID consoleMainArea = ImGui::GetID("#consoleMainArea");
		ImGui::BeginChildFrame(consoleMainArea, ImVec2(0, height), ImGuiWindowFlags_NoBackground);

		if (filteredMessage.size() == 0) {
			ImGui::Text("No console messages found.");
		}
		else if (ImGui::BeginTable("consoleSplit", 2)) {
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, 24.0f);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			size_t i = 0;

			for (auto& msg : filteredMessage) {
				RenderMessage(++i, *msg);
			}

			ImGui::EndTable();
		}

		ImGui::EndChildFrame();

		ImGui::End();
	}
}

bool ConsolePanel::AddConsoleMessage(Grindstone::Events::BaseEvent* ev) {
	auto printMsg = (Events::PrintMessageEvent*)ev;
	for (auto& message : messageQueue) {
		if (message.base.internalType == printMsg->message.internalType &&
			message.base.message == printMsg->message.message) {
			message.count++;
			return true;
		}
	}

	std::string& srcMsg = printMsg->message.message;
	std::string newTextLowercase;
	newTextLowercase.resize(srcMsg.size());
	std::transform(
		srcMsg.begin(),
		srcMsg.end(),
		newTextLowercase.begin(),
		::tolower
	);

	messageQueue.emplace_front(printMsg->message, newTextLowercase, 1);

	while (messageQueue.size() > 200) {
		messageQueue.pop_back();
	}

	FilterSearch();

	return true;
}
