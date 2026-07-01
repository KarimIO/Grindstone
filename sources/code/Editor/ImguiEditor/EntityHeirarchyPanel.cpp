#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>

#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <Editor/Commands/EntityCommands.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/ImguiEditor/ViewportPanel.hpp>
#include <Editor/EditorCamera.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>

#include "ImguiEditor.hpp"
#include "EntityHeirarchyPanel.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::ImguiEditor;

ECS::Entity entityToRename;
std::string entityRenameNewName;

struct ChildEntity {
	entt::entity childEntity;
	TagComponent* tagComponent;
	ParentComponent* parentComponent;

	ChildEntity(entt::entity childEntity, TagComponent* tagComponent, ParentComponent* parentComponent)
		: childEntity(childEntity), tagComponent(tagComponent), parentComponent(parentComponent) {
	}
};

static void RenderWorldContextSet(Grindstone::WorldContextSet* cxtSet);
static void RenderEntity(
	EntityParentTagView& view,
	Grindstone::ECS::Entity entity,
	Grindstone::TagComponent& tagComponent,
	Grindstone::ParentComponent& parentComponent
);

EntityHeirarchyPanel::EntityHeirarchyPanel(
	ImguiEditor* editor
) : editor(editor) {}

void EntityHeirarchyPanel::Render() {
	if (isShowingPanel) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Entity Heirarchy", &isShowingPanel);

		// BeginChild is used as a DropTarget for use of unparenting entities.
		// We begin a new child because windows can't be drop targets
		ImGui::BeginChild("Entity Heirarchy DropTarget", ImVec2(0,0), false, ImGuiWindowFlags_NoDocking);

		if (
			ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
			ImGui::IsWindowHovered() &&
			!ImGui::GetIO().KeyCtrl
		) {
			Editor::Manager::GetInstance().GetSelection().Clear();
		}

		Grindstone::WorldContextSet* worldContextSet = Grindstone::EngineCore::GetInstance().GetWorldContextManager()->GetActiveWorldContextSet();
		RenderWorldContextSet(worldContextSet);

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

static void RenderWorldContextSet(Grindstone::WorldContextSet* cxtSet) {
	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::MenuItem("Add new entity")) {
			Editor::Manager::GetInstance().GetCommandList().AddNewEntity(cxtSet);
		}
		ImGui::EndPopup();
	}

	entt::registry& registry = cxtSet->GetEntityRegistry();
	bool hasEntities = false;

	EntityParentTagView view = registry.view<entt::entity, TagComponent, ParentComponent>();

	view.each(
		[&cxtSet, &view, &hasEntities](
			entt::entity entity,
			TagComponent& tagComponent,
			ParentComponent& parentComponent
		) {
			if (parentComponent.parentEntity == entt::null) {
				RenderEntity(view, { entity, cxtSet }, tagComponent, parentComponent);
				hasEntities = true;
			}
		}
	);

	if (!hasEntities) {
		ImGui::Text("No entities in world.");
	}
}

static void RenderEntity(
	EntityParentTagView& view,
	Grindstone::ECS::Entity entity,
	Grindstone::TagComponent& tagComponent,
	Grindstone::ParentComponent& parentComponent
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

	if (ImGui::IsItemHovered()) {
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			Grindstone::TransformComponent& transform = entity.GetComponent<Grindstone::TransformComponent>();
			Grindstone::Editor::Manager& editor = Grindstone::Editor::Manager::GetInstance();
			Grindstone::Editor::EditorCamera* editorCamera = editor.GetImguiEditor().GetViewportPanel()->GetCamera();
			glm::vec3 newPosition = transform.position + (editorCamera->GetForward() * -2.0f);
			editorCamera->SetPosition(newPosition);
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
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
	}
	
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Rename")) {
			entityToRename = entity;
			entityRenameNewName = entityTag;
		}
		if (ImGui::MenuItem("Delete")) {
			selection.RemoveEntity(entity);
			entity.Destroy();
		}
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();

	ImGui::PopStyleColor();

	if (isOpened) {
		for (ChildEntity& child : children) {
			RenderEntity(view, { child.childEntity, entity.GetWorldContextSet() }, *child.tagComponent, *child.parentComponent);
		}

		ImGui::TreePop();
	}
}
