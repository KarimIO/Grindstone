#include <imgui.h>
#include <vector>

#include <Editor/EditorManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/Materials/MaterialImporter.hpp>
#include <EngineCore/Assets/Textures/TextureImporter.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineImporter.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineImporter.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <Plugins/Renderables3D/Assets/Mesh3dImporter.hpp>
#include <Plugins/Renderables3D/Mesh3dRenderer.hpp>

#include "StatsPanel.hpp"

using namespace Grindstone::Editor::ImguiEditor;
using namespace Grindstone::Memory;

struct MemoryDumpRow {
	std::string rowName;
	size_t size;
	void* pointer;
	size_t offset;
};

struct MemoryDumpData {
	bool hasCapturedMemoryDump;
	std::vector<MemoryDumpRow> rows;
} memoryDumpData;

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

static void RenderRenderQueuesTable(Grindstone::EngineCore& engineCore) {
	if (!ImGui::TreeNode("Render Queues")) {
		return;
	}

	Grindstone::AssetRendererManager* assetRendererManager = engineCore.assetRendererManager;
	Grindstone::Mesh3dRenderer* meshRenderer = nullptr;
	for (auto& assetRenderer : assetRendererManager->assetRenderers) {
		Grindstone::Mesh3dRenderer* testMeshRenderer = static_cast<Grindstone::Mesh3dRenderer*>(assetRenderer.second);
		if (testMeshRenderer != nullptr) {
			meshRenderer = testMeshRenderer;
			break;
		}
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
		const size_t oneKB = 1024u;
		const size_t memPeak = AllocatorCore::GetPeak() / oneKB;
		const size_t memUsed = AllocatorCore::GetUsed() / oneKB;
		const size_t memTotal = AllocatorCore::GetTotal() / oneKB;
		const float memUsedPct = static_cast<float>(memUsed) / static_cast<float>(memTotal);
		ImGui::Text("Peak: %zuKB", memPeak);
		ImGui::Text("Memory Used: %zuKB / %zuKB", memUsed, memTotal);
		ImGui::ProgressBar(memUsedPct, ImVec2(maxWidth, 0));

		// TODO: Investigate how we can re-add this for Release.
#if _DEBUG
		if (ImGui::Button("Capture Memory Dump")) {
			memoryDumpData.hasCapturedMemoryDump = true;

			const auto& nameMap = AllocatorCore::GetAllocatorState()->dynamicAllocator.GetNameMap();

			memoryDumpData.rows.reserve(nameMap.size());

			using AllocatorHeader = Memory::Allocators::DynamicAllocator::AllocationHeader;
			char* memStart = static_cast<char*>(AllocatorCore::GetAllocatorState()->dynamicAllocator.GetMemory());
			for (const auto& allocation : nameMap) {
				AllocatorHeader* header = reinterpret_cast<AllocatorHeader*>(static_cast<char*>(allocation.first) - sizeof(AllocatorHeader));
				size_t offset = static_cast<size_t>(static_cast<char*>(allocation.first) - memStart);
				memoryDumpData.rows.emplace_back(MemoryDumpRow{ allocation.second, header->blockSize, allocation.first, offset });
			}
		}

		if (memoryDumpData.hasCapturedMemoryDump)
		{
			if (ImGui::Button("Export Memory Dump")) {
				std::ofstream outputFile(engineCore.GetProjectPath() / "log" / "MemoryDump.csv");
				outputFile << "Allocation Name,\tSize,\tOffset\n";
				for (MemoryDumpRow& memoryRow : memoryDumpData.rows) {
					outputFile << memoryRow.rowName << ",\t" << memoryRow.size << ",\t" << memoryRow.offset << '\n';
				}
				outputFile.close();
			}

			if (ImGui::TreeNode("Memory Allocations"))
			{
				if (ImGui::BeginTable("statsImporterSplit", 3)) {
					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn("Ptr", ImGuiTableColumnFlags_WidthFixed);

					ImGui::TableHeadersRow();

					size_t i = 0;
					for (MemoryDumpRow& memoryRow : memoryDumpData.rows) {
						bool isEven = (++i % 2) == 0;
						ImGuiCol_ colorKey = isEven ? ImGuiCol_TableRowBg : ImGuiCol_TableRowBgAlt;
						ImU32 color = ImGui::GetColorU32(colorKey);
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, color);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", memoryRow.rowName.c_str());
						ImGui::TableNextColumn();
						ImGui::Text("%lu", memoryRow.size);
						ImGui::TableNextColumn();
						ImGui::Text("%p", memoryRow.pointer);
					}

					ImGui::EndTable();
				}

				ImGui::TreePop();
			}
		}
#endif // #if _DEBUG
	}

	ImGui::Separator();

	Assets::AssetManager* assetManager = engineCore.assetManager;

	RenderRenderQueuesTable(engineCore);

	RenderAsset<Mesh3dImporter>(assetManager, "3D Meshes");
	RenderAsset<MaterialImporter>(assetManager, "Materials");
	RenderAsset<ComputePipelineImporter>(assetManager, "Compute Pipeline Sets");
	RenderAsset<GraphicsPipelineImporter>(assetManager, "Graphics Pipeline Sets");
	RenderAsset<TextureImporter>(assetManager, "Textures");
}
