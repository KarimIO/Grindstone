#if 0
#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
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
		GPRINT_ERROR(LogSource::Editor, "Failed to initialize OpenGL");
		return;
	}
	if (!gl3wIsSupported(3, 2)) {
		GPRINT_ERROR(LogSource::Editor, "OpenGL 3.2 not supported\n");
		return;
	}

	ImGui_ImplOpenGL3_Init("#version 150");
}

CommandBuffer* ImguiRendererOpenGL::GetCommandBuffer() {
	return nullptr;
}

bool ImguiRendererOpenGL::PreRender() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	glViewport(0, 0, 800, 600);
	glClear(GL_COLOR_BUFFER_BIT);
	return true;
}

void ImguiRendererOpenGL::PrepareImguiRendering() {

}

void ImguiRendererOpenGL::PostRender() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void ImguiRendererOpenGL::Resize() {

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
#endif
