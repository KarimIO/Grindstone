#include <imgui.h>

#include <Editor/EditorManager.hpp>

#include "imgui_widget_flamegraph.h"

#include "TracingPanel.hpp"

using namespace Grindstone::Editor::ImguiEditor;

static void ValuesGetter(
	float* start,
	float* end,
	ImU8* level,
	const char** caption,
	const void* data,
	int idx
) {
	const float sToMs = 1000.0f;
	const Grindstone::Profiler::InstrumentationSession* session = reinterpret_cast<const Grindstone::Profiler::InstrumentationSession*>(data);
	const Grindstone::Profiler::Result& stage = session->results[idx];

	if (start != nullptr) {
		*start = static_cast<float>(stage.start) * sToMs;
	}

	if (end != nullptr) {
		*end = static_cast<float>(stage.end) * sToMs;
	}

	if (caption != nullptr) {
		*caption = stage.name.c_str();
	}

	if (level != nullptr) {
		*level = stage.depth - 1;
	}
}

void TracingPanel::Render() {
	if (isShowingPanel) {
		ImGui::Begin("Tracing", &isShowingPanel);
		TryCaptureSession();
		RenderContents();
		ImGui::End();
	}
}

void TracingPanel::TryCaptureSession() {
	const long long secondsToRecapture = 3;

	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	long long elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastCaptureTime).count();
	if (elapsedSeconds <= secondsToRecapture) {
		return;
	}

	lastCaptureTime = currentTime;
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	Profiler::Manager* profiler = engineCore.GetProfiler();

	session = profiler->GetAvailableSession();
}

void TracingPanel::RenderContents() {
	float containerWidth = ImGui::GetContentRegionAvail().x;

	ImGuiWidgetFlameGraph::PlotFlame(
		"##CPU Profiling",
		&ValuesGetter,
		&session,
		session.results.size(),
		0,
		"Main Thread",
		FLT_MAX,
		FLT_MAX,
		ImVec2(containerWidth, 100)
	);
}
