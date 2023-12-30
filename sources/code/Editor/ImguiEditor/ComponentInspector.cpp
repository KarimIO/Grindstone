#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>
#include <glm/gtc/quaternion.hpp>
#include "ComponentInspector.hpp"
#include "Editor/EditorManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/Reflection/TypeDescriptor.hpp"
#include "EngineCore/Assets/Asset.hpp"
#include "Plugins/ScriptCSharp/Components/ScriptComponent.hpp"
#include <Editor/ImguiEditor/ImguiEditor.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>
#include "Common/Math.hpp"

using namespace Grindstone::Editor::ImguiEditor;

ComponentInspector::ComponentInspector(ImguiEditor* editor) : imguiEditor(editor) {}

void ComponentInspector::Render(ECS::Entity entity) {
	ImGui::Text("Entity ID: %u", (uint32_t)entity.GetHandle());

	TagComponent* tagComponent = nullptr;
	if (entity.TryGetComponent<TagComponent>(tagComponent)) {
		ImGui::InputText(
			"Tag Name",
			&tagComponent->tag
		);
	}

	auto& editorManager = Editor::Manager::GetInstance();
	auto& engineCore = editorManager.GetEngineCore();
	ECS::ComponentRegistrar& componentRegistrar = *engineCore.GetComponentRegistrar();
	std::vector<std::string> unusedComponentsItems;
	std::vector<ECS::ComponentFunctions> unusedComponentsFunctions;
	for (auto componentEntry : componentRegistrar) {
		const char* componentTypeName = componentEntry.first.c_str();
		auto componentReflectionData = componentEntry.second.GetComponentReflectionDataFn();
		auto tryGetComponentFn = componentEntry.second.TryGetComponentFn;

		void* outComponent = nullptr;
		if (entity.TryGetComponent(componentTypeName, outComponent)) {
			RenderComponent(componentTypeName, componentReflectionData, outComponent, entity);
		}
		else {
			unusedComponentsItems.push_back(componentEntry.first);
			unusedComponentsFunctions.push_back(componentEntry.second);
		}
	}

	ImGui::Separator();

	ImGui::Text("Attach a component:");
	newComponentInput.Render(
		entity,
		unusedComponentsItems
	);
}

void ComponentInspector::RenderComponent(
	const char* componentTypeName,
	Grindstone::Reflection::TypeDescriptor_Struct& componentReflectionData,
	void* componentPtr,
	ECS::Entity entity
) {
	if (strcmp(componentTypeName, "Parent") == 0 || strcmp(componentTypeName, "Tag") == 0) {
		// Don't render these
		return;
	}

	if (!ImGui::TreeNodeEx(componentTypeName, ImGuiTreeNodeFlags_FramePadding)) {
		return;
	}

	if (strcmp(componentTypeName, "CSharpScript") == 0) {
		RenderCSharpScript(componentPtr, entity);
	}
	else {
		RenderComponentCategory(componentReflectionData.category, componentPtr, entity);
	}

	ImGui::TreePop();
}

void ComponentInspector::RenderCSharpScript(
	void* componentPtr,
	ECS::Entity entity
) {
	auto component = static_cast<Grindstone::Scripting::CSharp::ScriptComponent*>(componentPtr);

	if (component == nullptr || component->monoClass == nullptr) {
		return;
	}

	ImGui::Text("%s", component->monoClass->scriptClassname.c_str());
	size_t i = 0;
	for (auto& field : component->monoClass->fields) {
		float val = 0.0f;
		field.second.Get((MonoObject*)component->scriptObject, &val);
		if (ImGui::InputFloat(field.first.c_str(), &val)) {
			field.second.Set((MonoObject*)component->scriptObject, &val);
		}
	}
}

void ComponentInspector::RenderComponentCategory(
	Reflection::TypeDescriptor_Struct::Category& category,
	void* componentPtr,
	ECS::Entity entity
) {
	for (auto& subcategory : category.categories) {
		if (!ImGui::TreeNodeEx(subcategory.name.c_str(), ImGuiTreeNodeFlags_FramePadding)) {
			return;
		}

		RenderComponentCategory(subcategory, componentPtr, entity);

		ImGui::TreePop();
	}

	for (auto& member : category.members) {
		RenderComponentMember(member, componentPtr, entity);
	}
}

void ComponentInspector::RenderComponentMember(
	Reflection::TypeDescriptor_Struct::Member& member,
	void* componentPtr,
	ECS::Entity entity
) {
	void* offset = ((char*)componentPtr + member.offset);
	RenderComponentMember(std::string_view(member.displayName), member.type, offset, entity);
}

void ComponentInspector::RenderComponentMember(std::string_view displayName, Reflection::TypeDescriptor* itemType, void* offset, ECS::Entity entity) {
	const char* displayNamePtr = displayName.data();

	switch (itemType->type) {
	case Reflection::TypeDescriptor::ReflectionTypeData::Bool:
		ImGui::Checkbox(
			displayNamePtr,
			(bool*)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::AssetReference: {
		auto assetManager = Editor::Manager::GetEngineCore().assetManager;

		auto assetReferenceType = static_cast<Reflection::TypeDescriptor_AssetReference*>(itemType);
		GenericAssetReference* assetReference = (GenericAssetReference*)offset;

		AssetType assetType = assetReferenceType->assetType;
		Uuid uuid = assetReference->uuid;
		std::string name = "Unassigned";
		AssetRegistry::Entry entry;
		if (Editor::Manager::GetInstance().GetAssetRegistry().TryGetAssetData(uuid, entry)) {
			name = entry.name;
		}

		std::string buttonText = (name + "##" + displayNamePtr);
		ImVec2 buttonSize = { ImGui::GetContentRegionAvail().x, 24 };
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
		if (ImGui::Button(buttonText.c_str(), buttonSize)) {
			std::function<void(Uuid, std::string)> callback = [assetManager, assetReference, assetType](Uuid newUuid, std::string path) {
				assetReference->uuid = newUuid;
				};

			imguiEditor->PromptAssetPicker(assetType, callback);
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GetAssetTypeToString(assetReferenceType->assetType))) {
				Uuid uuid = *static_cast<Uuid*>(payload->Data);
				AssetRegistry::Entry entry;
				if (Editor::Manager::GetInstance().GetAssetRegistry().TryGetAssetData(uuid, entry)) {
					assetReference->uuid = uuid;
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleVar();
		break;
	}
	case Reflection::TypeDescriptor::ReflectionTypeData::Entity: {
		entt::entity& targetEntity = *(entt::entity*)offset;

		std::string name = "Unassigned";

		entt::registry& registry = entity.GetSceneEntityRegistry();

		if (targetEntity != entt::null) {
			Grindstone::TagComponent& tagComponent = registry.get<Grindstone::TagComponent>(targetEntity);
			if (!tagComponent.tag.empty()) {
				name = tagComponent.tag;
			}
		}

		std::string buttonText = (name + "##" + displayNamePtr);
		ImVec2 buttonSize = { ImGui::GetContentRegionAvail().x, 24 };
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
		if (ImGui::Button(buttonText.c_str(), buttonSize)) {
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
				ECS::Entity newTargetEntity = *static_cast<ECS::Entity*>(payload->Data);
				entt::entity newTargetEntityHandle = newTargetEntity.GetHandle();
				if (newTargetEntity.GetSceneEntityRegistry().valid(newTargetEntityHandle)) {
					targetEntity = newTargetEntityHandle;
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleVar();
		break;
	}
	case Reflection::TypeDescriptor::ReflectionTypeData::String:
		ImGui::InputText(
			displayNamePtr,
			(std::string *)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Int:
		ImGui::InputInt(
			displayNamePtr,
			(int*)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Int2:
		ImGui::InputInt2(
			displayNamePtr,
			(int *)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Int3:
		ImGui::InputInt3(
			displayNamePtr,
			(int *)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Int4:
		ImGui::InputInt4(
			displayNamePtr,
			(int *)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Float:
		ImGui::InputFloat(
			displayNamePtr,
			(float*)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Float2:
		ImGui::InputFloat2(
			displayNamePtr,
			(float *)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Float3:
		ImGui::InputFloat3(
			displayNamePtr,
			(float *)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Float4:
		ImGui::InputFloat4(
			displayNamePtr,
			(float*)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Quaternion: {
		glm::quat* quaternion = (glm::quat*)offset;
		glm::vec3 euler = glm::degrees(glm::eulerAngles(*quaternion));
		if (ImGui::InputFloat3(
			displayNamePtr,
			&euler[0]
		)) {
			*quaternion = glm::quat(glm::radians(euler));
		}
		break;
	}
	case Reflection::TypeDescriptor::ReflectionTypeData::Double:
		ImGui::InputDouble(
			displayNamePtr,
			(double*)offset
		);
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Vector:
		ImGui::Text(displayNamePtr);
		ImGui::SameLine();
		const void* vector = static_cast<const void*>(offset);
		auto vectorType = static_cast<Reflection::TypeDescriptor_StdVector*>(itemType);
		size_t vectorSize = vectorType->getSize(offset);

		std::string buttonFieldName = std::string("+##") + displayNamePtr;
		if (ImGui::Button(buttonFieldName.c_str())) {
			vectorType->emplaceBack(offset);
		}

		size_t itemToDelete = -1;
		for (size_t i = 0; i < vectorSize; ++i) {
			std::string fieldName = std::string("##") + std::to_string(i) + displayNamePtr;
			RenderComponentMember(std::string_view(fieldName), vectorType->itemType, vectorType->getItem(offset, i), entity);
			ImGui::SameLine();
			std::string eraseFieldName = std::string("-") + fieldName;
			if (ImGui::Button(eraseFieldName.c_str())) {
				itemToDelete = i;

				// TODO: Handle effects of remove?
			}
		}

		if (itemToDelete != -1) {
			vectorType->erase(offset, itemToDelete);
		}

		break;
	}
}
