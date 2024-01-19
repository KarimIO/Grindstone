#include <imgui.h>
#include "Editor/EditorManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "StatsPanel.hpp"

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

	void StatsPanel::RenderContents() {
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
	}
}
