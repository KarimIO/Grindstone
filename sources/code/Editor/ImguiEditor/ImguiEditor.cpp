#include <iostream>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <Windows.h>
#include <Winuser.h>
#include "GL/gl3w.h"
#include "EngineCore/EngineCore.hpp"
#include "ImguiEditor.hpp"
#include "SystemPanel.hpp"
#include "SceneHeirarchyPanel.hpp"
using namespace Grindstone::Editor::ImguiEditor;

bool demoWindowIsShown = true;
ImGuiID dockspaceId;

ImguiEditor::ImguiEditor(EngineCore* engineCore) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	if (gl3wInit()) {
		fprintf(stderr, "failed to initialize OpenGL\n");
		return;
	}
	if (!gl3wIsSupported(3, 2)) {
		fprintf(stderr, "OpenGL 3.2 not supported\n");
		return;
	}

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	HWND win = GetActiveWindow();
	ImGui_ImplWin32_Init(win);
	ImGui_ImplOpenGL3_Init("#version 150");

	sceneHeirarchyPanel = new SceneHeirarchyPanel(engineCore->getSceneManager());
	systemPanel = new SystemPanel(engineCore->getSystemRegistrar());
}

void ImguiEditor::update() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	glViewport(0, 0, 800, 600);
	glClear(GL_COLOR_BUFFER_BIT);

	dockspaceId = ImGui::GetID("MyDockspace");
	ImGui::DockSpace(dockspaceId);

	render();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void ImguiEditor::render() {
	sceneHeirarchyPanel->render();
	systemPanel->render();
}
