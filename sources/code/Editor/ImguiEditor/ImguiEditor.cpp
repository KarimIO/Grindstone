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
#include "InspectorPanel.hpp"
#include "SceneHeirarchyPanel.hpp"
#include "Menubar.hpp"
#include "ImguiInput.hpp"
using namespace Grindstone::Editor::ImguiEditor;

ImguiEditor::ImguiEditor(EngineCore* engineCore) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


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

	HWND win = GetActiveWindow();
	ImGui_ImplWin32_Init(win);
	
	input = new ImguiInput(io, engineCore);
	
	ImGui_ImplOpenGL3_Init("#version 150");

	sceneHeirarchyPanel = new SceneHeirarchyPanel(engineCore->getSceneManager(), this);
	inspectorPanel = new InspectorPanel(engineCore);
	systemPanel = new SystemPanel(engineCore->getSystemRegistrar());
	menubar = new Menubar();
}

void ImguiEditor::update() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	glViewport(0, 0, 800, 600);
	glClear(GL_COLOR_BUFFER_BIT);
	
	render();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void ImguiEditor::render() {
	renderDockspace();
	sceneHeirarchyPanel->render();
	systemPanel->render();
	inspectorPanel->render(selectedEntity);
}

void ImguiEditor::updateSelectedEntity(entt::entity selectedEntity) {
	this->selectedEntity = selectedEntity;
}

void ImguiEditor::renderDockspace() {
	static bool opt_fullscreen_persistant = true;
	static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
	bool optFullscreen = opt_fullscreen_persistant;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// - because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (optFullscreen) 	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and
	// - handle the pass-thru hole, so we ask Begin() to not render a background.
	if (opt_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Editor Dockspace", nullptr, window_flags);
	ImGui::PopStyleVar();

	if (optFullscreen)
		ImGui::PopStyleVar(2);

	// Dockspace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) 	{
		ImGuiID dockspaceId = ImGui::GetID("Editor Dockspace");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), opt_flags);
	}

	menubar->render();

	ImGui::End();
}
