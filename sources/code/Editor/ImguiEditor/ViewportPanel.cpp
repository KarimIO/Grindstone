#include <imgui/imgui.h>
#include "ViewportPanel.hpp"
#include "../EditorCamera.hpp"
using namespace Grindstone::Editor::ImguiEditor;

ViewportPanel::ViewportPanel(GraphicsAPI::Core* graphicsCore) {
	camera = new EditorCamera(graphicsCore);
}

void ViewportPanel::render() {
	if (isShowingPanel) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", &isShowingPanel);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		camera->ResizeViewport(viewportPanelSize.x, viewportPanelSize.y);
		camera->Render();

		int textureID = camera->GetPrimaryFramebufferAttachment();
		auto texturePtr = reinterpret_cast<void*>(textureID);
		auto size = ImVec2{ viewportPanelSize.x, viewportPanelSize.y };
		auto uv0 = ImVec2{ 0, 1 };
		auto uv1 = ImVec2{ 1, 0 };
		ImGui::Image(texturePtr, size, uv0, uv1);

		ImGui::End();
		ImGui::PopStyleVar();
	}
}
