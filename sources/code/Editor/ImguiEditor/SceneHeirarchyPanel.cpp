#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>
#include "EngineCore/Scenes/Manager.hpp"
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
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Scene Heirarchy", &isShowingPanel);

		// BeginChild is used as a DropTarget for use of unparenting entities.
		// We begin a new child because windows can't be drop targets
		ImGui::BeginChild("Scene Heirarchy DropTarget", ImVec2(0,0), false, ImGuiWindowFlags_NoDocking);

		if (
			ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
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
				SceneManagement::Scene* scene = scenePair.second;
				const char* sceneName = scene->GetName().c_str();
				if (ImGui::TreeNode(sceneName)) {
					RenderScene(scene);
					ImGui::TreePop();
				}
			}
		}

		ImGui::EndChild();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
				ECS::Entity newTargetEntity = *static_cast<ECS::Entity*>(payload->Data);
				newTargetEntity.SetParent(ECS::Entity());
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}
}

void SceneHeirarchyPanel::RenderScene(SceneManagement::Scene* scene) {
	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::MenuItem("Add new entity")) {
			Editor::Manager::GetInstance().GetCommandList().AddNewEntity(scene);
		}
		ImGui::EndPopup();
	}

	entt::registry& registry = scene->GetEntityRegistry();
	bool hasEntities = false;

	EntityParentTagView view = registry.view<entt::entity, TagComponent, ParentComponent>();

	view.each(
		[&](
			entt::entity entity,
			TagComponent& tagComponent,
			ParentComponent& parentComponent
		) {
			if (parentComponent.parentEntity == entt::null) {
				RenderEntity(view, { entity, scene }, tagComponent, parentComponent);
				hasEntities = true;
			}
		}
	);

	if (!hasEntities) {
		ImGui::Text("No entities in scene.");
	}
}

struct ChildEntity {
	entt::entity childEntity;
	TagComponent* tagComponent;
	ParentComponent* parentComponent;

	ChildEntity(entt::entity childEntity, TagComponent* tagComponent, ParentComponent* parentComponent)
		: childEntity(childEntity), tagComponent(tagComponent), parentComponent(parentComponent)
	{

	}
};

void SceneHeirarchyPanel::RenderEntity(
	EntityParentTagView& view,
	ECS::Entity entity,
	TagComponent& tagComponent,
	ParentComponent& parentComponent
) {
	const float panelWidth = ImGui::GetContentRegionAvail().x;
	ImGui::PushItemWidth(panelWidth);

	entt::entity entityHandle = entity.GetHandle();

	std::vector<ChildEntity> children;
	view.each(
		[&](
			entt::entity childEntity,
			TagComponent& tagComponent,
			ParentComponent& parentComponent
		) {
			if (parentComponent.parentEntity == entityHandle) {
				children.emplace_back(childEntity, &tagComponent, &parentComponent);
			}
		}
	);

	bool isLeaf = children.empty();

	Grindstone::Editor::Selection& selection = Editor::Manager::GetInstance().GetSelection();
	bool isSelected = selection.IsEntitySelected(entity);
	auto& colors = ImGui::GetStyle().Colors;
	ImGui::PushStyleColor(ImGuiCol_Header, colors[ImGuiCol_Button]);

	const char* entityTag = tagComponent.tag.c_str();

	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0, 0.5});

	ImGuiTreeNodeFlags treeFlags =
		ImGuiTreeNodeFlags_SpanAvailWidth |
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_DefaultOpen |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		(isLeaf
			? ImGuiTreeNodeFlags_Leaf
			: 0
		) |
		(isSelected
			? ImGuiTreeNodeFlags_Selected
			: 0
		);

	bool isOpened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entityHandle, treeFlags, entityTag);

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
			entt::registry& registry = entity.GetSceneEntityRegistry();
			ECS::Entity newTargetEntity = *static_cast<ECS::Entity*>(payload->Data);
			newTargetEntity.SetParent(entity);
		}

		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("Entity", &entity, sizeof(ECS::Entity));
		ImGui::Text("%s", entityTag);
		ImGui::EndDragDropSource();
	}

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

	ImGui::PopStyleColor();

	if (isOpened) {
		for (ChildEntity& child : children) {
			RenderEntity(view, { child.childEntity, entity.GetScene() }, *child.tagComponent, *child.parentComponent);
		}

		ImGui::TreePop();
	}
}
