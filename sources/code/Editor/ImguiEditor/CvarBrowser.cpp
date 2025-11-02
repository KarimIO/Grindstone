#include <algorithm>
#include <unordered_map>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <Common/Console/Cvars.hpp>

#include "CvarBrowser.hpp"

using namespace Grindstone::Editor::ImguiEditor;

static void Label(const char* label, float textWidth) {
	constexpr float slack = 50;
	constexpr float editorWidth = 100;

	const ImVec2 lineStart = ImGui::GetCursorScreenPos();
	const ImGuiStyle& style = ImGui::GetStyle();
	float fullWidth = textWidth + slack;

	ImVec2 textSize = ImGui::CalcTextSize(label);

	ImVec2 startPos = ImGui::GetCursorScreenPos();

	ImGui::Text(label);

	ImVec2 finalPos = { startPos.x + fullWidth, startPos.y };

	ImGui::SameLine();
	ImGui::SetCursorScreenPos(finalPos);

	ImGui::SetNextItemWidth(editorWidth);
}

static void RenderFieldValue(Grindstone::CvarSystem* cvarSystem, Grindstone::CvarParameter* p, float textWidth) {
	const bool readonlyFlag = ((uint32_t)p->flags & (uint32_t)Grindstone::CvarFlags::ReadOnlyUi);
	const bool checkboxFlag = ((uint32_t)p->flags & (uint32_t)Grindstone::CvarFlags::Boolean);
	const bool dragFlag = ((uint32_t)p->flags & (uint32_t)Grindstone::CvarFlags::EditorNumberSlider);

	Label(p->name.c_str(), textWidth);

	switch (p->type) {
	case Grindstone::CvarType::Float:
		ImGui::PushID(p->name.c_str());
		ImGui::InputDouble("", cvarSystem->GetFloatCvar(p->arrayIndex), 0, 0, "%.3f");
		ImGui::PopID();
		break;
	case Grindstone::CvarType::Integer:
		ImGui::PushID(p->name.c_str());
		ImGui::InputInt("", cvarSystem->GetIntCvar(p->arrayIndex));
		ImGui::PopID();
		break;
	case Grindstone::CvarType::String:
		ImGui::PushID(p->name.c_str());
		ImGui::InputText("", cvarSystem->GetStringCvar(p->arrayIndex));
		ImGui::PopID();
		break;
	case Grindstone::CvarType::Undefined:
		ImGui::Text("Invalid");
		break;
	}

	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(p->description.c_str());
	}
}

CvarBrowser::CvarBrowser() : isShowingPanel(false) {}

void CvarBrowser::Render() {
	if (isShowingPanel) {
		return;
	}

	ImGui::Begin("Cvar Browser", &isShowingPanel);

	static std::string searchText = "";

	ImGui::InputText("Filter", &searchText);
	static bool shouldShowAdvanced = false;
	ImGui::Checkbox("Advanced", &shouldShowAdvanced);
	ImGui::Separator();
	cachedEditParameters.clear();

	auto addToEditList = [this](Grindstone::CvarParameter& parameter) {
		uint32_t flags = static_cast<uint32_t>(parameter.flags);
		bool bHidden = (flags & static_cast<uint32_t>(Grindstone::CvarFlags::Hidden));
		bool bIsAdvanced = (flags & static_cast<uint32_t>(Grindstone::CvarFlags::Advanced));

		if (!bHidden &&
			!(!shouldShowAdvanced && bIsAdvanced) &&
			(searchText.empty() || parameter.name.find(searchText) != std::string::npos)
		) {
			cachedEditParameters.push_back(&parameter);
		};
	};

	CvarSystem* cvarSystem = CvarSystem::GetInstance();

	for (auto it = cvarSystem->begin(); it != cvarSystem->end(); it++) {
		addToEditList(it->second);
	}

	std::unordered_map<std::string, std::vector<Grindstone::CvarParameter*>> categorizedParams;

	for (auto p : cachedEditParameters) {
		int dotPos = -1;
		for (int i = 0; i < p->name.length(); i++) {
			if (p->name[i] == '.') {
				dotPos = i;
				break;
			}
		}
		std::string category = "";
		if (dotPos != -1) {
			category = p->name.substr(0, dotPos);
		}

		auto it = categorizedParams.find(category);
		if (it == categorizedParams.end()) {
			categorizedParams[category] = std::vector<Grindstone::CvarParameter*>();
			it = categorizedParams.find(category);
		}

		it->second.push_back(p);
	}

	for (auto& [category, parameters] : categorizedParams) {
		std::sort(
			parameters.begin(),
			parameters.end(),
			[](Grindstone::CvarParameter* A, Grindstone::CvarParameter* B) {
				return A->name < B->name;
			}
		);

		if (ImGui::TreeNode(category.c_str())) {
			float maxTextWidth = 0;

			for (auto p : parameters) {
				maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(p->name.c_str()).x);
			}

			for (auto p : parameters) {
				RenderFieldValue(cvarSystem, p, maxTextWidth);
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
}
