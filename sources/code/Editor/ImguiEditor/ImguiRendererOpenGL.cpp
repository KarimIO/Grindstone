#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <GL/gl3w.h>
#include <Windows.h>
#include <Winuser.h>

#include <Editor/EditorManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/Textures/TextureAsset.hpp>
#include <Plugins/GraphicsOpenGL/GLTexture.hpp>

#include "ImguiRendererOpenGL.hpp"

using namespace Grindstone::Editor::ImguiEditor;

ImguiRendererOpenGL::ImguiRendererOpenGL() {
	HWND win = GetActiveWindow();
	ImGui_ImplWin32_Init(win);

	if (gl3wInit()) {
		Editor::Manager::Print(LogSeverity::Error, "Failed to initialize OpenGL");
		return;
	}
	if (!gl3wIsSupported(3, 2)) {
		Editor::Manager::Print(LogSeverity::Error, "OpenGL 3.2 not supported\n");
		return;
	}

	ImGui_ImplOpenGL3_Init("#version 150");
}

void ImguiRendererOpenGL::PreRender() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	glViewport(0, 0, 800, 600);
	glClear(GL_COLOR_BUFFER_BIT);
}

void ImguiRendererOpenGL::PostRender() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

ImTextureID ImguiRendererOpenGL::CreateTexture(std::filesystem::path path) {
	std::filesystem::path fullPath = "../engineassets/editor/" / path;
	auto assetManager = Editor::Manager::GetEngineCore().assetManager;
	auto textureAsset = static_cast<TextureAsset*>(assetManager->GetAsset(Grindstone::AssetType::Texture, fullPath.string().c_str()));

	if (textureAsset == nullptr) {
		return 0;
	}

	GraphicsAPI::GLTexture* glTex = static_cast<GraphicsAPI::GLTexture*>(textureAsset->texture);
	return (ImTextureID)(uint64_t)glTex->GetTexture();
}
