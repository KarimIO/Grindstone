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

namespace Grindstone::Physics {
	class JoltPhysicsSettingsPage : public Grindstone::Editor::ImguiEditor::Settings::BasePage {
	public:
		virtual void Open() {
			Reset();
		}

		virtual void Render() {
			size_t layerToErase = SIZE_MAX;

			ImGui::Text("Constant Values");
			if (ImGui::BeginTable("SettingsPageSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX)) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Gravity");
				ImGui::TableNextColumn();
				ImGui::InputFloat3("##GravityValue", &gravity[0]);

				ImGui::EndTable();
			}

			ImGui::Separator();
			ImGui::NewLine();

			ImGui::Text("Layers");
			if (layerNames.size() == 0) {
				ImGui::Text("No layerNames available.");
			}
			else {
				if (ImGui::BeginTable("SettingsPageSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX)) {
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("Layer 0");
					ImGui::TableNextColumn();
					ImGui::Text(layerNames[0].c_str());

					for (size_t layerIndex = 1; layerIndex < layerNames.size(); ++layerIndex) {
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						std::string layerName = "Layer " + std::to_string(layerIndex);
						ImGui::Text(layerName.c_str());
						ImGui::TableNextColumn();
						std::string layerStringName = "##" + layerName;
						ImGui::InputText(layerStringName.c_str(), &layerNames[layerIndex]);
						ImGui::SameLine();
						std::string layerStringRemoveBtnName = "-##" + layerName;
						if (ImGui::Button(layerStringRemoveBtnName.c_str())) {
							layerToErase = layerIndex;
						}
					}

					ImGui::EndTable();
				}
			}

			if (layerNames.size() == 32) {
				ImGui::Text("Grindstone supports a maximum of 32 layerNames.");
			}
			else {
				if (ImGui::Button("Add Layer")) {
					layerNames.emplace_back("Unnamed Layer");
				}
			}

			ImGui::Separator();
			ImGui::NewLine();

			ImGui::Text("Layer Collision Matrix");
			if (layerNames.size() == 0) {
				ImGui::Text("No layerNames available.");
			}
			else {
				if (ImGui::BeginTable("SettingsPageSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX)) {
					for (size_t layerIndex = 0; layerIndex < layerNames.size(); ++layerIndex) {
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text(layerNames[layerIndex].c_str());
						ImGui::TableNextColumn();
						for (size_t cellIndex = 0; cellIndex < (layerNames.size() - layerIndex); ++cellIndex) {
							std::string cellName = "##layerMatrixCell_" + std::to_string(layerIndex) + "_" + std::to_string(cellIndex);
							ImGui::Checkbox(cellName.c_str(), &layerMatrix[layerIndex][cellIndex]);
							ImGui::SameLine();
						}
					}

					ImGui::EndTable();
				}
			}

			if (layerToErase != SIZE_MAX) {
				layerNames.erase(layerNames.begin() + layerToErase);
			}
		}

		virtual void Save() {

		}

		virtual void Reset() {
			layerNames.resize(1);
			layerNames[0] = "Default";

			gravity = glm::vec3(0.0f, -9.81f, 0.0f);

			for (size_t i = 0; i < 32; ++i) {
				for (size_t j = 0; j < 32; ++j) {
					layerMatrix[i][j] = false;
				}
			}
		}

	public:

		std::vector<std::string> layerNames;
		bool layerMatrix[32][32];
		glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);

	};
}

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
