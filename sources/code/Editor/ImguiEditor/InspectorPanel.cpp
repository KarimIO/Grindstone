#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>

#include <Common/ResourcePipeline/MetaFile.hpp>
#include <Editor/AssetRegistry.hpp>
#include <Editor/EditorManager.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/Scenes/Scene.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "ComponentInspector.hpp"
#include "MaterialInspector.hpp"
#include "InspectorPanel.hpp"

using namespace Grindstone::Memory;
using namespace Grindstone::Editor::ImguiEditor;

struct InspectorState {
	std::filesystem::path lastSelectedAssetPath;
	void* importerMenuPayload = nullptr;
} inspectorState;

static void RenderGenericFile(const std::filesystem::path& path) {
	static std::string newAddress;
	static Uuid selectedEntryToEdit;

	Grindstone::Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Grindstone::Editor::AssetRegistry::Entry entry;
	if (assetRegistry.TryGetAssetDataFromAbsolutePath(path, entry)) {
		if (ImGui::TreeNode(entry.displayName.c_str())) {
			ImGui::Text("Path: %s", entry.path.string().c_str());
			ImGui::Text("Asset Type: %s", GetAssetTypeToString(entry.assetType));
			ImGui::Text("Display Name: %s", entry.displayName.c_str());
			if (selectedEntryToEdit == entry.uuid) {
				ImGui::InputText("Address", &newAddress);
				if (ImGui::Button("Save")) {
					Grindstone::Editor::MetaFile metaFile(assetRegistry, path);
					Grindstone::Editor::MetaFile::Subasset* subasset = nullptr;
					if (metaFile.TryGetSubasset(entry.uuid, subasset)) {
						subasset->address = newAddress;
					}
					metaFile.SaveWithoutImporterVersionChange();
					assetRegistry.WriteFile();

					selectedEntryToEdit = Uuid();
				}
			}
			else {
				ImGui::Text("Address: %s", entry.address.c_str());
				if (ImGui::Button("Edit")) {
					newAddress = entry.address;
					selectedEntryToEdit = entry.uuid;
				}
			}
			ImGui::TreePop();
		}
	}
	else {
		ImGui::Text("Unmanaged file: %s", path.string().c_str());
	}
}

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
	else if (!inspectorState.lastSelectedAssetPath.empty()) {
		Grindstone::Editor::Manager& editorInstance = Grindstone::Editor::Manager::GetInstance();
		Grindstone::Importers::ImporterManager& importerManager = editorInstance.GetImporterManager();
		Grindstone::Editor::ImporterData lastImporterData = importerManager.GetImporterFactoryByPath(inspectorState.lastSelectedAssetPath);

		if (lastImporterData.onMenuCleanup != nullptr) {
			lastImporterData.onMenuCleanup(inspectorState.importerMenuPayload);
		}
	}
}

void InspectorPanel::RenderContents() {
	Grindstone::Editor::Manager& editorInstance = Grindstone::Editor::Manager::GetInstance();
	Selection& selection = editorInstance.GetSelection();
	Grindstone::Importers::ImporterManager& importerManager = editorInstance.GetImporterManager();

	bool hasHandledSelected = false;
	size_t selectedEntityCount = selection.GetSelectedEntityCount();
	size_t selectedFileCount = selection.GetSelectedFileCount();

	if (selectedEntityCount == 0 && selectedFileCount == 0) {
		ImGui::Text("Nothing selected.");

		if (!inspectorState.lastSelectedAssetPath.empty()) {
			Grindstone::Editor::ImporterData lastImporterData = importerManager.GetImporterFactoryByPath(inspectorState.lastSelectedAssetPath);

			if (lastImporterData.onMenuCleanup != nullptr) {
				lastImporterData.onMenuCleanup(inspectorState.importerMenuPayload);
			}
			inspectorState.lastSelectedAssetPath = "";
		}
		return;
	}
	else if (selectedEntityCount == 0 && selectedFileCount > 0) {
		if (selectedFileCount == 1) {
			const std::filesystem::path& path = selection.GetSingleSelectedFile();
			RenderGenericFile(path);
			std::string extension = path.extension().string();

			Grindstone::Editor::ImporterData importerData = importerManager.GetImporterFactoryByPath(path);
			if (inspectorState.lastSelectedAssetPath != path) {
				if (!inspectorState.lastSelectedAssetPath.empty()) {
					Grindstone::Editor::ImporterData lastImporterData = importerManager.GetImporterFactoryByPath(inspectorState.lastSelectedAssetPath);
					if (lastImporterData.onMenuCleanup != nullptr) {
						lastImporterData.onMenuCleanup(inspectorState.importerMenuPayload);
					}
					inspectorState.lastSelectedAssetPath = "";
				}

				if (!path.empty()) {
					if (importerData.onMenuStart != nullptr) {
						inspectorState.importerMenuPayload = importerData.onMenuStart(path);
					}
					inspectorState.lastSelectedAssetPath = path;
				}
			}

			if (extension == ".gmat") {
				materialInspector->SetMaterialPath(path);
				materialInspector->Render();
			}
			else if (importerData.onMenuRender != nullptr) {
				importerData.onMenuRender(inspectorState.importerMenuPayload);
			}
			return;
		}

		if (!inspectorState.lastSelectedAssetPath.empty()) {
			Grindstone::Editor::ImporterData lastImporterData = importerManager.GetImporterFactoryByPath(inspectorState.lastSelectedAssetPath);

			if (lastImporterData.onMenuCleanup != nullptr) {
				lastImporterData.onMenuCleanup(inspectorState.importerMenuPayload);
			}

			inspectorState.lastSelectedAssetPath = "";
		}

		unsigned int unmanagedFiles = 0;
		for (const auto& path : selection.selectedFiles) {
			RenderGenericFile(path);
		}
		return;
	}
	else if (selectedFileCount == 0 && selectedEntityCount == 1) {
		componentInspector->Render(selection.GetSingleSelectedEntity());
		return;
	}

	if (selectedEntityCount > 0) {
		ImGui::Text("%i entities selected.", selectedEntityCount);
	}

	if (selectedFileCount > 0) {
		ImGui::Text("%i files selected.", selectedFileCount);
	}
}
