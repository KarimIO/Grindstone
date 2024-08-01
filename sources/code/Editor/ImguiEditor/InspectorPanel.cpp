#include <imgui.h>
#include <entt/entt.hpp>

#include <Editor/EditorManager.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/Scenes/Scene.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "ComponentInspector.hpp"
#include "MaterialInspector.hpp"
#include "InspectorPanel.hpp"

using namespace Grindstone::Memory;

namespace Grindstone::Editor::ImguiEditor {
	InspectorPanel::InspectorPanel(EngineCore* engineCore, ImguiEditor* imguiEditor) : engineCore(engineCore) {
		componentInspector = AllocatorCore::Allocate<ComponentInspector>(imguiEditor);
		materialInspector = AllocatorCore::Allocate<MaterialInspector>(engineCore, imguiEditor);
	}

	InspectorPanel::~InspectorPanel() {
		AllocatorCore::Free(materialInspector);
		AllocatorCore::Free(componentInspector);
	}

	void InspectorPanel::Render() {
		if (isShowingPanel) {
			ImGui::Begin("Inspector", &isShowingPanel);
			RenderContents();
			ImGui::End();
		}
	}

	void InspectorPanel::RenderContents() {
		bool hasHandledSelected = false;
		Selection& selection = Editor::Manager::GetInstance().GetSelection();
		size_t selectedEntityCount = selection.GetSelectedEntityCount();
		size_t selectedFileCount = selection.GetSelectedFileCount();

		if (selectedEntityCount == 0 && selectedFileCount == 1) {
			auto entry = selection.GetSingleSelectedFile();
			if (!entry.is_directory()) {
				auto& stringPath = entry.path();
				std::string extension = stringPath.extension().string();

				if (extension == ".gmat") {
					materialInspector->SetMaterialPath(stringPath);
					materialInspector->Render();
					return;
				}
			}
		}
		else if (selectedEntityCount == 1 && selectedFileCount == 0) {
			componentInspector->Render(selection.GetSingleSelectedEntity());
			return;
		}

		if (selectedEntityCount > 0 || selectedFileCount > 0) {
			if (selectedEntityCount > 0) {
				ImGui::Text("%i entities selected.", selectedEntityCount);
			}

			if (selectedFileCount > 0) {
				ImGui::Text("%i files selected.", selectedFileCount);
			}

			return;
		}

		ImGui::Text("Nothing selected.");
	}
}
