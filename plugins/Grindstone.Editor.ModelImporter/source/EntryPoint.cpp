#include <imgui.h>

#include <Common/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include <Grindstone.Editor.ModelImporter/include/pch.hpp>
#include <Grindstone.Editor.ModelImporter/include/ModelImporter.hpp>
#include <Grindstone.Editor.ModelImporter/include/ModelMaterialImporter.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Editor::Importers;

Grindstone::ConstHashedString modelImporterName = "ModelImporter";

struct ModelImporterRendererState {
	Grindstone::Editor::MetaFile metaFile;
	std::filesystem::path path;
	bool shouldImportScene;
	bool shouldImportLights;
	bool shouldImportCameras;
	bool shouldFlipUvs;
	bool shouldReduceDuplicateMeshes;
	bool shouldFlipFaces;
	bool shouldOptimizeMeshes;
	bool shouldOptimizeScene;
	bool shouldSplitLargeMeshes;
	float scale;
};

static void* OnSetupModelImporterRenderer(const std::filesystem::path& path) {
	ModelImporterRendererState* payload = Grindstone::Memory::AllocatorCore::Allocate<ModelImporterRendererState>();
	payload->metaFile = Grindstone::Editor::Manager::GetInstance().GetAssetRegistry().GetMetaFileByPath(path);
	payload->path = path;

	Grindstone::Editor::ImporterSettings& settings = payload->metaFile.GetImporterSettings();
	payload->shouldImportScene = settings.Get("ImportScene", true);
	payload->shouldImportLights = settings.Get("ImportLights", true);
	payload->shouldImportCameras = settings.Get("ImportCameras", true);
	payload->shouldFlipUvs = settings.Get("FlipUVs", true);
	payload->shouldReduceDuplicateMeshes = settings.Get("ReduceDuplicateMeshes", true);
	payload->shouldFlipFaces = settings.Get("FlipFaces", false);
	payload->shouldOptimizeMeshes = settings.Get("OptimizeMeshes", true);
	payload->shouldOptimizeScene = settings.Get("OptimizeScene", true);
	payload->shouldSplitLargeMeshes = settings.Get("SplitLargeMeshes", false);
	payload->scale = static_cast<float>(settings.Get("Scale", 1.0f));

	return payload;
}

static void OnRenderModelImporterRenderer(void* payload) {
	GS_ASSERT(payload != nullptr);
	ModelImporterRendererState* state = reinterpret_cast<ModelImporterRendererState*>(payload);

	ImGui::DragFloat("Scale", &state->scale, 0.1f, 0.1f, 1000000.0f);

	ImGui::Checkbox("Import Scene", &state->shouldImportScene);
	ImGui::Checkbox("Import Lights", &state->shouldImportLights);
	ImGui::Checkbox("Import Cameras", &state->shouldImportCameras);
	ImGui::Checkbox("Flip UVs", &state->shouldFlipUvs);
	ImGui::Checkbox("Flip Faces", &state->shouldFlipFaces);

	ImGui::Checkbox("Reduce Duplicate Meshes", &state->shouldReduceDuplicateMeshes);
	ImGui::Checkbox("Optimize Scene", &state->shouldOptimizeScene);
	ImGui::Checkbox("Optimize Meshes", &state->shouldOptimizeMeshes);
	ImGui::Checkbox("Split Large Meshes", &state->shouldSplitLargeMeshes);

	if (ImGui::Button("Save")) {
		Grindstone::Editor::ImporterSettings& settings = state->metaFile.GetImporterSettings();
		settings.Set("ImportScene", state->shouldImportScene);
		settings.Set("ImportLights", state->shouldImportLights);
		settings.Set("ImportCameras", state->shouldImportCameras);
		settings.Set("FlipUVs", state->shouldFlipUvs);
		settings.Set("ReduceDuplicateMeshes", state->shouldReduceDuplicateMeshes);
		settings.Set("FlipFaces", state->shouldFlipFaces);
		settings.Set("OptimizeMeshes", state->shouldOptimizeMeshes);
		settings.Set("OptimizeScene", state->shouldOptimizeScene);
		settings.Set("SplitLargeMeshes", state->shouldSplitLargeMeshes);
		settings.Set("Scale", static_cast<double>(state->scale));
		state->metaFile.Save(modelImporterVersion);
	}
}

static void OnCleanupModelImporterRenderer(void* payload) {
	GS_ASSERT(payload != nullptr);

	ModelImporterRendererState* state = reinterpret_cast<ModelImporterRendererState*>(payload);
	Grindstone::Memory::AllocatorCore::Free(state);
}

extern "C" {
	EDITOR_MODEL_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		ImGui::SetCurrentContext(editorPluginInterface->GetImguiContext());
		Grindstone::Editor::Manager::SetInstance(editorPluginInterface->GetEditorInstance());
		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter(modelImporterName, Grindstone::Editor::ImporterData{
				.importerVersion = modelImporterVersion,
				.factory = ImportModel,
				.onMenuStart = OnSetupModelImporterRenderer,
				.onMenuRender = OnRenderModelImporterRenderer,
				.onMenuCleanup = OnCleanupModelImporterRenderer
			});
			editorPluginInterface->MapExtensionToImporterType("fbx", modelImporterName);
			editorPluginInterface->MapExtensionToImporterType("dae", modelImporterName);
			editorPluginInterface->MapExtensionToImporterType("obj", modelImporterName);
		}
	}

	EDITOR_MODEL_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->UnmapExtensionToImporterType("fbx");
			editorPluginInterface->UnmapExtensionToImporterType("dae");
			editorPluginInterface->UnmapExtensionToImporterType("obj");
			editorPluginInterface->DeregisterAssetImporter(modelImporterName);
		}
	}
}
