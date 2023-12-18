#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "Editor/Commands/EntityCommands.hpp"
#include "Editor/EditorManager.hpp"
#include "ImguiEditor.hpp"
#include "SceneHeirarchyPanel.hpp"

using namespace Grindstone::Editor::ImguiEditor;

SceneHeirarchyPanel::SceneHeirarchyPanel(
	SceneManagement::SceneManager* sceneManager,
	ImguiEditor* editor
) : sceneManager(sceneManager), editor(editor) {}

void SceneHeirarchyPanel::Render() {
	if (isShowingPanel) {
		ImGui::Begin("Scene Heirarchy", &isShowingPanel);

		if (
			ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
			ImGui::IsWindowHovered() &&
			!ImGui::GetIO().KeyCtrl
		) {
			Editor::Manager::GetInstance().GetSelection().Clear();
		}

		auto numScenes = sceneManager->scenes.size();
		if (numScenes == 0) {
			ImGui::Text("No scenes mounted.");
		}
		else if (numScenes == 1) {
			auto sceneIterator = sceneManager->scenes.begin();
			RenderScene(sceneIterator->second);
		}
		else {
			for (auto& scenePair : sceneManager->scenes) {
				auto* scene = scenePair.second;
				const char* sceneName = scene->GetName();
				if (ImGui::TreeNode(sceneName)) {
					RenderScene(scene);
					ImGui::TreePop();
				}
			}
		}

		ImGui::End();
	}
}

const char* SceneHeirarchyPanel::GetEntityTag(ECS::Entity entity) {
	if (entity.HasComponent<TagComponent>()) {
		return entity.GetComponent<TagComponent>().tag.c_str();
	}

	return "[Unnamed Entity]";
}

void SceneHeirarchyPanel::RenderScene(SceneManagement::Scene* scene) {
	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::MenuItem("Add new entity")) {
			Editor::Manager::GetInstance().GetCommandList().AddNewEntity(scene);
		}
		ImGui::EndPopup();
	}

	auto& registry = scene->GetEntityRegistry();
	auto& entityStorage = registry.storage<entt::entity>();

	if (entityStorage.in_use()) {
		ImGui::Text("No entities in scene.");
	}
	else {
		for (const entt::entity entity : entityStorage) {
			RenderEntity({ entity, scene });
		}
	}
}

void SceneHeirarchyPanel::RenderEntity(ECS::Entity entity) {
	const float panelWidth = ImGui::GetContentRegionAvail().x;
	ImGui::PushItemWidth(panelWidth);

	Selection& selection = Editor::Manager::GetInstance().GetSelection();
	bool isSelected = selection.IsEntitySelected(entity);
	auto& colors = ImGui::GetStyle().Colors;
	ImGui::PushStyleColor(
		ImGuiCol_Button,
		isSelected ? colors[ImGuiCol_Button] : ImVec4(1, 1, 1, 0.05f)
	);
	ImGui::PushStyleColor(
		ImGuiCol_ButtonHovered,
		isSelected ? colors[ImGuiCol_ButtonHovered] : ImVec4(1, 1, 1, 0.15f)
	);
	ImGui::PushStyleColor(
		ImGuiCol_ButtonActive,
		isSelected ? colors[ImGuiCol_ButtonActive] : ImVec4(1, 1, 1, 0.2f)
	);

	const char* entityTag = GetEntityTag(entity);
	if (entityToRename == entity) {
		const auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
		if (ImGui::InputText("##AssetRename", &entityRenameNewName, flags)) {
			entityToRename = ECS::Entity();
			TagComponent* tagComponent = nullptr;
			if (entity.TryGetComponent<TagComponent>(tagComponent)) {
				tagComponent->tag = entityRenameNewName;
			}
			entityRenameNewName = "";
		}
	}
	else {
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0, 0.5});
		ImGui::Button(entityTag, { panelWidth, 0 });
		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			if (ImGui::GetIO().KeyCtrl) {
				if (selection.IsEntitySelected(entity)) {
					selection.RemoveEntity(entity);
				}
				else {
					selection.AddEntity(entity);
				}
			}
			else {
				selection.SetSelectedEntity(entity);
			}
		}

		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Rename")) {
				entityToRename = entity;
				entityRenameNewName = entityTag;
			}
			if (ImGui::MenuItem("Delete")) {
				selection.RemoveEntity(entity);
				entity.GetScene()->DestroyEntity(entity);
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
	}

	ImGui::PopStyleColor(3);
}
