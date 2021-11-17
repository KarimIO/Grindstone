#pragma once

#include <chrono>

namespace Grindstone {
	class EngineCore;
	namespace Editor {
		namespace ImguiEditor {
			class StatsPanel {
			public:
				StatsPanel();
				void Render();
			private:
				void RenderContents();
				bool isShowingPanel = true;
				unsigned long long totalFrameCount = 0;
				unsigned int frameCountSinceLastRender = 0;
				std::chrono::steady_clock::time_point lastRenderTime;


				double lastRenderedDeltaTime = 0.0;
				unsigned long long lastRenderedTotalFrameCount = 0;
			};
		}
	}
}
