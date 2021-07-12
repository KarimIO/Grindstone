#include <iostream>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <Windows.h>
#include <Winuser.h>
#include "GL/gl3w.h"

#include "Common/Window/WindowManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Modals/ModelConverterModal.hpp"
#include "Modals/ImageConverterModal.hpp"
#include "ViewportPanel.hpp"
#include "ImguiEditor.hpp"
#include "SystemPanel.hpp"
#include "InspectorPanel.hpp"
#include "AssetBrowserPanel.hpp"
#include "SceneHeirarchyPanel.hpp"
#include "Menubar.hpp"
#include "ImguiInput.hpp"
using namespace Grindstone::Editor::ImguiEditor;

ImguiEditor::ImguiEditor(EngineCore* engineCore) : engineCore(engineCore) {

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
	modelConverterModal = new ModelConverterModal();
	imageConverterModal = new ImageConverterModal();
	inspectorPanel = new InspectorPanel(engineCore);
	assetBrowserPanel = new AssetBrowserPanel(engineCore, this);
	viewportPanel = new ViewportPanel(engineCore->getGraphicsCore());
	systemPanel = new SystemPanel(engineCore->getSystemRegistrar());
	menubar = new Menubar(this);
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
	modelConverterModal->render();
	imageConverterModal->render();
	assetBrowserPanel->render();
	sceneHeirarchyPanel->render();
	viewportPanel->render();
	systemPanel->render();
	inspectorPanel->render();
}

void ImguiEditor::showModelModal() {
	modelConverterModal->show();
}

void ImguiEditor::showImageModal() {
	imageConverterModal->show();
}

void ImguiEditor::deselectFromInspector() {
	inspectorPanel->deselect();
}

void ImguiEditor::selectFile(std::string selectedFileType, std::string selectedFilePath) {
	inspectorPanel->selectFile(selectedFileType, selectedFilePath);
}

void ImguiEditor::selectEntity(entt::entity selectedEntity) {
	this->selectedEntity = selectedEntity;
	inspectorPanel->selectEntity(selectedEntity);
}

void ImguiEditor::importFile(const char* folderPathToImportTo) {
	auto window = engineCore->windowManager->GetWindowByIndex(0);
	auto filePath = window->OpenFileDialogue();
}

void ImguiEditor::renderDockspace() {
	static ImGuiDockNodeFlags optFlags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// - because it would be confusing to have two docking targets within each others.
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and
	// - handle the pass-thru hole, so we ask Begin() to not render a background.
	if (optFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		windowFlags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Editor Dockspace", nullptr, windowFlags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	ImGuiID dockspaceId = ImGui::GetID("Editor Dockspace");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), optFlags);

	menubar->render();

	ImGui::End();
}
