#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>
#include <glm/gtc/quaternion.hpp>

#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/Reflection/TypeDescriptor.hpp>
#include <EngineCore/Assets/Asset.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/ImguiEditor/ImguiEditor.hpp>
#include <Plugins/ScriptCSharp/Components/ScriptComponent.hpp>
#include <Common/Math.hpp>

#include "ComponentInspector.hpp"

using namespace Grindstone::Editor::ImguiEditor;

const Grindstone::ConstHashedString parentComponentName("Parent");
const Grindstone::ConstHashedString tagComponentName("Tag");
const Grindstone::ConstHashedString csharpComponentName("CSharpScript");

/*
#define RenderMonoFieldType(Type, FunctionName) { \
		Type val; \
		mono_field_get_value(monoObject, classField, &val); \
		if (FunctionName(fieldName, &val)) { \
			mono_field_set_value(monoObject, classField, &val); \
		} \
		break; \
	}

static void RenderMonoField(MonoObject* monoObject, MonoClassField* classField) {
	const char* fieldName = mono_field_get_name(classField);
	MonoType* monoType = mono_field_get_type(classField);
	int monoTypeActual = mono_type_get_type(monoType);

	switch (monoTypeActual) {
	case MONO_TYPE_BOOLEAN: RenderMonoFieldType(bool, ImGui::Checkbox);
	case MONO_TYPE_I4: RenderMonoFieldType(int, ImGui::InputInt);
	case MONO_TYPE_R4: RenderMonoFieldType(float, ImGui::InputFloat);
	case MONO_TYPE_R8: RenderMonoFieldType(double, ImGui::InputDouble);
	}
}
*/

static bool DrawFloatInput(const char* name, float& toEdit, size_t index, float containerWidth) {
	constexpr const char* fields[] = { "X", "Y", "Z" };
	constexpr ImVec4 regularColors[] = {
		ImVec4{ 0.6f, 0.1f, 0.1f, 1.0f },
		ImVec4{ 0.05f, 0.5f, 0.05f, 1.0f },
		ImVec4{ 0.1f, 0.1f, 0.55f, 1.0f }
	};
	constexpr ImVec4 hoverColors[] = {
		ImVec4{ 0.6f, 0.1f, 0.1f, 1.0f },
		ImVec4{ 0.05f, 0.5f, 0.05f, 1.0f },
		ImVec4{ 0.1f, 0.1f, 0.55f, 1.0f }
	};
	constexpr ImVec4 pressColors[] = {
		ImVec4{ 0.6f, 0.1f, 0.1f, 1.0f },
		ImVec4{ 0.05f, 0.5f, 0.05f, 1.0f },
		ImVec4{ 0.1f, 0.1f, 0.55f, 1.0f }
	};

	const float spacingX = 8.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ spacingX, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 2.0f });

	ImGui::PushStyleColor(ImGuiCol_Button, regularColors[index]);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, pressColors[index]);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColors[index]);
	constexpr float framePadding = 2.0f;
	constexpr float outlineSpacing = 1.0f;
	float fontSize = ImGui::GetFontSize();
	float lineHeight = fontSize + framePadding * 2.0f;
	ImVec2 buttonSize = { lineHeight + 2.0f, lineHeight };
	const float inputItemWidth = containerWidth - buttonSize.x - outlineSpacing * 2.0f;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(framePadding, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1.0f);
	ImGui::Button((fields[index] + std::string("##") + name).c_str(), buttonSize);
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);
	ImGui::SameLine(0.0f, 0.1f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - framePadding / 2.0f);
	ImGui::PushItemWidth(inputItemWidth);
	bool hasChanged = ImGui::InputFloat((std::string("##") + name + fields[index]).c_str(), &toEdit);
	ImGui::PopItemWidth();
	ImGui::PopStyleVar(2);

	return hasChanged;
}

static void DrawFloatInputNewLine() {
	ImGui::SameLine(0.0f, 0.2f);
}

static bool DrawFloat2(const char* name, float* vec2) {
	float containerWidth = ImGui::GetContentRegionAvail().x / 2.0f;
	bool hasChanged = DrawFloatInput(name, vec2[0], 0, containerWidth);
	DrawFloatInputNewLine();
	hasChanged |= DrawFloatInput(name, vec2[1], 1, containerWidth);

	return hasChanged;
}

static bool DrawFloat3(const char* name, float* vec3) {
	float containerWidth = ImGui::GetContentRegionAvail().x / 3.0f;
	bool hasChanged = DrawFloatInput(name, vec3[0], 0, containerWidth);
	DrawFloatInputNewLine();
	hasChanged |= DrawFloatInput(name, vec3[1], 1, containerWidth);
	DrawFloatInputNewLine();
	hasChanged |= DrawFloatInput(name, vec3[2], 2, containerWidth);

	return hasChanged;
}

static bool DrawFloat4(const char* name, float* vec4) {
	float containerWidth = ImGui::GetContentRegionAvail().x / 4.0f;
	bool hasChanged = DrawFloatInput(name, vec4[0], 0, containerWidth);
	DrawFloatInputNewLine();
	hasChanged |= DrawFloatInput(name, vec4[1], 1, containerWidth);
	DrawFloatInputNewLine();
	hasChanged |= DrawFloatInput(name, vec4[2], 2, containerWidth);
	DrawFloatInputNewLine();
	hasChanged |= DrawFloatInput(name, vec4[3], 3, containerWidth);

	return hasChanged;
}

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

	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	EngineCore& engineCore = editorManager.GetEngineCore();
	ECS::ComponentRegistrar& componentRegistrar = *engineCore.GetComponentRegistrar();
	std::vector<std::string> unusedComponentsItems;
	std::vector<ECS::ComponentFunctions> unusedComponentsFunctions;
	for (auto& componentEntry : componentRegistrar) {
		Grindstone::HashedString componentTypeName = componentEntry.first;
		auto componentReflectionData = componentEntry.second.GetComponentReflectionDataFn();
		auto tryGetComponentFn = componentEntry.second.TryGetComponentFn;

		void* outComponent = nullptr;
		if (entity.TryGetComponent(componentTypeName, outComponent)) {
			RenderComponent(componentTypeName, componentReflectionData, outComponent, entity);
		}
		else {
			unusedComponentsItems.push_back(componentEntry.first.ToString());
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
	Grindstone::HashedString componentTypeName,
	Grindstone::Reflection::TypeDescriptor_Struct& componentReflectionData,
	void* componentPtr,
	ECS::Entity entity
) {
	if (componentTypeName == parentComponentName || componentTypeName == tagComponentName) {
		// Don't render these
		return;
	}

	if (componentTypeName == csharpComponentName) {
		RenderCSharpScript(componentPtr, entity);
	}
	else {
		bool shouldRemove = false;
		bool isOpened = ImGui::TreeNodeEx(componentTypeName.ToString().c_str(), ImGuiTreeNodeFlags_FramePadding);
		if (ImGui::BeginPopupContextItem(componentTypeName.ToString().c_str())) {
			if (ImGui::MenuItem("Remove Component")) {
				shouldRemove = true;
			}
			ImGui::EndPopup();
		}

		if (isOpened) {
			RenderComponentCategory(componentReflectionData.category, componentPtr, entity);
			ImGui::TreePop();
		}

		if (shouldRemove) {
			entity.RemoveComponent(componentTypeName);
		}
	}

}

void ComponentInspector::RenderCSharpScript(
	void* componentPtr,
	ECS::Entity entity
) {
	auto component = static_cast<Grindstone::Scripting::CSharp::ScriptComponent*>(componentPtr);

	if (component == nullptr) {
		return;
	}

	/*
	if (component->monoClass == nullptr) {
		bool shouldRemove = false;
		bool isOpened = ImGui::TreeNodeEx("Unassigned CSharp Component", ImGuiTreeNodeFlags_FramePadding);
		if (ImGui::BeginPopupContextItem("Unassigned CSharp Component")) {
			if (ImGui::MenuItem("Remove Component")) {
				shouldRemove = true;
			}
			ImGui::EndPopup();
		}

		if (isOpened) {
			ImGui::TreePop();
		}

		if (shouldRemove) {
			entity.RemoveComponent("CSharpScript");
		}

		return;
	}
	*/

	{
		bool shouldRemove = false;
		const std::string componentName = "(C#) " + component->scriptClass;
		bool isOpened = ImGui::TreeNodeEx(componentName.c_str(), ImGuiTreeNodeFlags_FramePadding);
		if (ImGui::BeginPopupContextItem(componentName.c_str())) {
			if (ImGui::MenuItem("Remove Component")) {
				shouldRemove = true;
			}
			ImGui::EndPopup();
		}

		if (isOpened) {
			// for (auto& field : component->monoClass->fields) {
				// RenderMonoField(component->scriptObject, field.second.classFieldPtr);
			// }

			ImGui::TreePop();
		}

		if (shouldRemove) {
			entity.RemoveComponent(csharpComponentName);
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

	if (ImGui::BeginTable("assetBrowserSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
		for (auto& member : category.members) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text(member.displayName.c_str());
			ImGui::TableNextColumn();
			RenderComponentMember(member, componentPtr, entity);
		}

		ImGui::EndTable();
	}
}

void ComponentInspector::RenderComponentMember(
	Reflection::TypeDescriptor_Struct::Member& member,
	void* componentPtr,
	ECS::Entity entity
) {
	void* offset = ((char*)componentPtr + member.offset);
	RenderComponentMember("##" + member.displayName, member.type, offset, entity);
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
		bool hasValue = uuid.IsValid();
		std::string name = "Unassigned";
		AssetRegistry::Entry entry;
		if (hasValue && Editor::Manager::GetInstance().GetAssetRegistry().TryGetAssetData(uuid, entry)) {
			name = entry.displayName;
		}

		std::string buttonText = (name + "##" + displayNamePtr);
		ImVec2 buttonSize = { ImGui::GetContentRegionAvail().x, 24 };
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
		if (ImGui::Button(buttonText.c_str(), buttonSize)) {
			std::function<void(Uuid, std::string)> callback = [assetManager, assetReference, assetType, hasValue](Uuid newUuid, std::string path) {
				// Handle old value
				if (hasValue) {
					assetManager->DecrementAssetCount(assetType, assetReference->uuid);
				}

				// Handle new value
				assetReference->uuid = newUuid;
				assetManager->IncrementAssetCount(assetType, newUuid);
			};

			imguiEditor->PromptAssetPicker(assetType, callback);
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GetAssetTypeToString(assetReferenceType->assetType))) {
				Uuid newUuid = *static_cast<Uuid*>(payload->Data);
				AssetRegistry::Entry entry;
				if (Editor::Manager::GetInstance().GetAssetRegistry().TryGetAssetData(newUuid, entry)) {
					// Handle old value
					if (hasValue) {
						assetManager->DecrementAssetCount(assetType, uuid);
					}

					// Handle new value
					assetReference->uuid = newUuid;
					assetManager->IncrementAssetCount(assetType, newUuid);
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
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::InputText(
			displayNamePtr,
			(std::string *)offset
		);
		ImGui::PopItemWidth();
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Int:
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::InputInt(
			displayNamePtr,
			(int*)offset
		);
		ImGui::PopItemWidth();
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
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::InputFloat(
			displayNamePtr,
			(float*)offset
		);
		ImGui::PopItemWidth();
		break;
	case Reflection::TypeDescriptor::ReflectionTypeData::Float2: {
		DrawFloat2(displayNamePtr, static_cast<float*>(offset));
		break;
	}
	case Reflection::TypeDescriptor::ReflectionTypeData::Float3: {
		DrawFloat3(displayNamePtr, static_cast<float*>(offset));
		break;
	}
	case Reflection::TypeDescriptor::ReflectionTypeData::Float4: {
		DrawFloat4(displayNamePtr, static_cast<float*>(offset));
		break;
	}
	case Reflection::TypeDescriptor::ReflectionTypeData::Quaternion: {
		glm::quat* quaternion = (glm::quat*)offset;
		glm::vec3 euler = glm::degrees(glm::eulerAngles(*quaternion));
		if (DrawFloat3(
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
