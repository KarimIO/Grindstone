#include <Grindstone.Physics.Jolt/include/pch.hpp>
#include <chrono>
#include <string>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Editor/PluginSystem/EditorPluginInterface.hpp>

#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsSystem.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;

#ifdef _WIN32
#ifdef PHYSICS_EDITOR_DLL
#define JOLT_PHYSICS_EDITOR_EXPORT __declspec(dllexport)
#else
#define JOLT_PHYSICS_EDITOR_EXPORT __declspec(dllimport)
#endif
#else
#define JOLT_PHYSICS_EDITOR_EXPORT
#endif

class JoltPhysicsSettingsPage : public Grindstone::Editor::ImguiEditor::Settings::BasePage {
public:
	virtual void Open() {
		layers.resize(1);
		layers[0] = "Default";
	}

	virtual void Render() {
		size_t layerToErase = SIZE_MAX;

		ImGui::Text("Constant Values");
		if (ImGui::BeginTable("SettingsPageSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Gravity");
			ImGui::TableNextColumn();
			ImGui::InputFloat("##GravityValue", &gravity);

			ImGui::EndTable();
		}

		ImGui::Separator();
		ImGui::NewLine();

		ImGui::Text("Layers");
		if (layers.size() == 0) {
			ImGui::Text("No layers available.");
		}
		else {
			if (ImGui::BeginTable("SettingsPageSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX)) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Layer 0");
				ImGui::TableNextColumn();
				ImGui::Text(layers[0].c_str());

				for (size_t layerIndex = 1; layerIndex < layers.size(); ++layerIndex) {
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					std::string layerName = "Layer " + std::to_string(layerIndex);
					ImGui::Text(layerName.c_str());
					ImGui::TableNextColumn();
					std::string layerStringName = "##" + layerName;
					ImGui::InputText(layerStringName.c_str(), &layers[layerIndex]);
					ImGui::SameLine();
					std::string layerStringRemoveBtnName = "-##" + layerName;
					if (ImGui::Button(layerStringRemoveBtnName.c_str())) {
						layerToErase = layerIndex;
					}
				}

				ImGui::EndTable();
			}
		}

		if (layers.size() == 32) {
			ImGui::Text("Grindstone supports a maximum of 32 layers.");
		}
		else {
			if (ImGui::Button("Add Layer")) {
				layers.emplace_back("Unnamed Layer");
			}
		}

		ImGui::Separator();
		ImGui::NewLine();

		ImGui::Text("Layer Collision Matrix");
		if (layers.size() == 0) {
			ImGui::Text("No layers available.");
		}
		else {
			if (ImGui::BeginTable("SettingsPageSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX)) {
				for (size_t layerIndex = 0; layerIndex < layers.size(); ++layerIndex) {
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text(layers[layerIndex].c_str());
					ImGui::TableNextColumn();
					for (size_t cellIndex = 0; cellIndex < (layers.size() - layerIndex); ++cellIndex) {
						std::string cellName = "##layerMatrixCell_" + std::to_string(layerIndex) + "_" + std::to_string(cellIndex);
						ImGui::Checkbox(cellName.c_str(), &layerMatrix[layerIndex][cellIndex]);
						ImGui::SameLine();
					}
				}

				ImGui::EndTable();
			}
		}

		if (layerToErase != SIZE_MAX) {
			layers.erase(layers.begin() + layerToErase);
		}
	}

	virtual void Save() {

	}

	virtual void Reset() {

	}

public:

	std::vector<std::string> layers;
	bool layerMatrix[32][32];
	float gravity = -9.81f;

};

extern "C" {
	JOLT_PHYSICS_EDITOR_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		ImGui::SetCurrentContext(editorPluginInterface->GetImguiContext());
		if (editorPluginInterface) {
			editorPluginInterface->RegisterProjectSettingsPage(
				"Jolt Physics",
				Grindstone::Memory::AllocatorCore::AllocateUnique<JoltPhysicsSettingsPage>()
			);
		}
	}

	JOLT_PHYSICS_EDITOR_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface) {
			editorPluginInterface->DeregisterProjectSettingsPage("Jolt Physics");
		}
	}
}
