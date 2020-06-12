#pragma once

#include <CoreEditor/EditorInstance.hpp>

#include <chrono>
#include <vector>

struct ImGuiWindowClass;
class Engine;
class DLLEngine;
class UICanvas;

namespace Grindstone {
	namespace GraphicsAPI {
		class GraphicsWrapper;
	}

	class EditorManager {
	public:
		EditorManager();
		~EditorManager();

		void initialize();
		void run();
		void cleanup();
	public:
		void addEditor(EditorInstance* editor);
		void openFile(std::string path);
		void dashboard();
	public:
		double getTimeCurrent();
		double getTimeDelta();
		//ImGuiWindowClass* getMainImguiWindowClass();
	private:
		void calculateTime();
	private:
		DLLEngine* dll_engine_;
		Engine* engine_;
		GraphicsAPI::GraphicsWrapper* graphics_wrapper_;
		//ImguiManager imgui_manager_;
		std::chrono::steady_clock::time_point begin, begin_frame, end_frame;
		double time_current_, time_delta_;

		// ImGuiWindowClass *main_imgui_window_class_;
		UICanvas* canvas;

		std::vector<EditorInstance*> editors_;
	};
}
