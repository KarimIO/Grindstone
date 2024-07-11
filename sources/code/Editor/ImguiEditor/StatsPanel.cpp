#include <imgui.h>
#include "Editor/EditorManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "StatsPanel.hpp"

#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/Assets/Textures/TextureImporter.hpp"
#include "EngineCore/Assets/Shaders/ShaderImporter.hpp"
#include "Plugins/Renderables3D/Assets/Mesh3dImporter.hpp"

#include "EngineCore/AssetRenderer/AssetRendererManager.hpp"
#include "Plugins/Renderables3D/Mesh3dRenderer.hpp"

namespace Grindstone::Editor::ImguiEditor {
	StatsPanel::StatsPanel() {
		lastRenderTime = std::chrono::steady_clock::now();
	}

	void StatsPanel::Render() {
		if (isShowingPanel) {
			ImGui::Begin("Stats", &isShowingPanel);
			RenderContents();
			ImGui::End();
		}
	}

	template<typename AssetImporterClass>
	void RenderAsset(Grindstone::Assets::AssetManager* assetManager, const char* title) {
		if (ImGui::TreeNode(title)) {
			AssetImporterClass* assetImporter = assetManager->GetManager<AssetImporterClass>();
			if (assetImporter == nullptr) {
				ImGui::Text("Importer unavailable.");
			}
			else if (!assetImporter->HasAssets()) {
				ImGui::Text("No assets imported.");
			}
			else if (ImGui::BeginTable("statsImporterSplit", 2)) {
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Uses", ImGuiTableColumnFlags_WidthFixed);

				ImGui::TableHeadersRow();

				size_t i = 0;
				for (auto& asset : *assetImporter) {
					bool isEven = (++i % 2) == 0;
					ImGuiCol_ colorKey = isEven ? ImGuiCol_TableRowBg : ImGuiCol_TableRowBgAlt;
					ImU32 color = ImGui::GetColorU32(colorKey);
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, color);
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("%s", asset.second.name.c_str());
					ImGui::TableNextColumn();
					ImGui::Text("%lu", asset.second.referenceCount);
				}

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	static void RenderRenderQueuesTable(EngineCore& engineCore) {
		if (!ImGui::TreeNode("Render Queues")) {
			return;
		}

		AssetRendererManager* assetRendererManager = engineCore.assetRendererManager;
		Mesh3dRenderer* meshRenderer = nullptr;
		for (auto& assetRenderer : assetRendererManager->assetRenderers) {
			Mesh3dRenderer* testMeshRenderer = static_cast<Mesh3dRenderer*>(assetRenderer.second);
			if (testMeshRenderer != nullptr) {
				meshRenderer = testMeshRenderer;
				break;
			}
		}

		if (meshRenderer == nullptr) {
			ImGui::TreePop();
			return;
		}

		if (ImGui::BeginTable("renderStatsSplit", 2)) {
			ImGui::TableSetupColumn("Render Queue", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Draw Count", ImGuiTableColumnFlags_WidthFixed);

			ImGui::TableHeadersRow();
			size_t i = 0;
			for (auto& queueMapValue : meshRenderer->renderQueueMap) {
				bool isEven = (++i % 2) == 0;
				ImGuiCol_ colorKey = isEven ? ImGuiCol_TableRowBg : ImGuiCol_TableRowBgAlt;
				ImU32 color = ImGui::GetColorU32(colorKey);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, color);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%s", queueMapValue.first.c_str());
				ImGui::TableNextColumn();
				RenderQueueIndex queueIndex = queueMapValue.second;
				RenderQueueContainer& queueContainer = meshRenderer->renderQueues[queueIndex];
				ImGui::Text("%lu", queueContainer.renderSortData.size());
			}

			ImGui::EndTable();
		}

		ImGui::TreePop();
	}

	void StatsPanel::RenderContents() {
		float maxWidth = ImGui::GetContentRegionAvail().x;

		totalFrameCount++;
		frameCountSinceLastRender++;

		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		long long elapsedTimeMicroSeconds = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastRenderTime).count();
		if (elapsedTimeMicroSeconds > 300000) {
			lastRenderTime = currentTime;
			lastRenderedTotalFrameCount = totalFrameCount;
			lastRenderedDeltaTime = elapsedTimeMicroSeconds / (1000000.0 * frameCountSinceLastRender);
			frameCountSinceLastRender = 0;
		}

		ImGui::Text("Total Frame Count: %I64u", totalFrameCount);
		ImGui::Text("Frame Time (Seconds): %f", lastRenderedDeltaTime);
		ImGui::Text("Frames Per Second: %f", 1 / lastRenderedDeltaTime);

		ImGui::Separator();

		EngineCore& engineCore = Editor::Manager::GetEngineCore();

		{
			const Memory::AllocatorCore& allocator = engineCore.GetAllocator();
			const size_t oneMB = 1024u * 1024u;
			const size_t memUsed = allocator.GetUsed() / oneMB;
			const size_t memTotal = allocator.GetTotal() / oneMB;
			const float memUsedPct = memUsed * 100.0f / memTotal;
			ImGui::Text("Memory Used: %zuMB / %zuMB", memUsed, memTotal);
			ImGui::ProgressBar(memUsedPct, ImVec2(maxWidth, 0));
		}

		ImGui::Separator();

		Assets::AssetManager* assetManager = engineCore.assetManager;

		RenderRenderQueuesTable(engineCore);

		RenderAsset<Mesh3dImporter>(assetManager, "3D Meshes");
		RenderAsset<MaterialImporter>(assetManager, "Materials");
		RenderAsset<ShaderImporter>(assetManager, "Shaders");
		RenderAsset<TextureImporter>(assetManager, "Textures");
	}
}
