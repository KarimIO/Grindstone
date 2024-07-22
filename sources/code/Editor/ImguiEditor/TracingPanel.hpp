#pragma once

#include <chrono>

#include <EngineCore/Profiling.hpp>

namespace Grindstone::Editor::ImguiEditor {
	class TracingPanel {
	public:
		void Render();
	private:
		void RenderContents();
		void TryCaptureSession();

		bool isShowingPanel = true;
		Profiler::InstrumentationSession session;
		std::chrono::steady_clock::time_point lastCaptureTime;
	};
}
