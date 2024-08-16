#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>

#include <Common/Event/PrintMessageEvent.hpp>
#include <Common/Event/EventType.hpp>

#include <EngineCore/Events/Dispatcher.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include <Editor/EditorManager.hpp>
#include "AssetPicker.hpp"
#include "ImguiRenderer.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::ImguiEditor;

const ImVec2 ASSET_PICKER_WINDOW_SIZE = { 300.f, 400.f };

void AssetPicker::OpenPrompt(AssetType assetType, AssetPickerCallback callback) {
	this->callback = callback;
	ImGui::OpenPopup("Asset Picker");

	assets.clear();
	filteredResults.clear();
	AssetRegistry& registry = Editor::Manager::GetInstance().GetAssetRegistry();
	registry.FindAllFilesOfType(assetType, assets);
	std::sort(
		assets.begin(), assets.end(),
		[](AssetRegistry::Entry& a, AssetRegistry::Entry& b) {
			return a.name < b.name;
		}
	);

	filteredResults.reserve(assets.size());
	for (size_t i = 0; i < assets.size(); ++i) {
		filteredResults.push_back(assets[i]);
	}
}

void AssetPicker::Render() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowSize(ASSET_PICKER_WINDOW_SIZE);

	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImVec2 windowPos = ImVec2(
		(displaySize.x - ASSET_PICKER_WINDOW_SIZE.x) / 2.0f,
		(displaySize.y - ASSET_PICKER_WINDOW_SIZE.y) / 2.0f
	);
	ImGui::SetNextWindowPos(windowPos);

	if (ImGui::BeginPopupModal("Asset Picker", &isShowing, flags)) {
		if (ImGui::InputText("Search", &searchString)) {
			std::string lowerSearch = Utils::ToLower(searchString);
			lowerSearch = Utils::TrimLeft(lowerSearch);

			filteredResults.clear();
			if (lowerSearch.size() == 0) {
				filteredResults.reserve(assets.size());
				for (size_t i = 0; i < assets.size(); ++i) {
					filteredResults.push_back(assets[i]);
				}
			}
			else {
				for (size_t i = 0; i < assets.size(); ++i) {
					const auto& asset = assets[i];
					std::string assetName = Utils::ToLower(asset.name);
					if (assetName.find(lowerSearch) != std::string::npos) {
						filteredResults.push_back(asset);
					}
				}
			}
		}

		if (filteredResults.size() == 0) {
			ImGui::Text("No related files found.");
			ImGui::EndPopup();
			return;
		}

		bool isInTable = ImGui::BeginTable("assetBrowserSplit", 1);

		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
		if (isInTable) {
			ImVec2 btnSize = { ImGui::GetContentRegionAvail().x, 22.0f };
			size_t i = 0;
			for (auto& asset : filteredResults) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(++i % 2 ? ImGuiCol_TableRowBgAlt : ImGuiCol_TableRowBg));
				ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);
				ImGui::TableNextColumn();
				if (ImGui::Button((asset.name + "##" + std::to_string(i)).c_str(), btnSize)) {
					callback(asset.uuid, asset.name);
					ImGui::CloseCurrentPopup();
					isShowing = false;
				}
			}

			ImGui::EndTable();
		}
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		ImGui::EndPopup();
	}
}
