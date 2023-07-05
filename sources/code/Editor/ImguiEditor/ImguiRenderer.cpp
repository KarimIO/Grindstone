#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Editor/EditorManager.hpp>

#include "ImguiRendererOpenGL.hpp"
#include "ImguiRendererVulkan.hpp"

#include "ImguiRenderer.hpp"

using namespace Grindstone::Editor::ImguiEditor;

ImguiRenderer* ImguiRenderer::Create() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	switch (graphicsCore->GetAPI()) {
		case Grindstone::GraphicsAPI::API::OpenGL:
			return new ImguiRendererOpenGL();
		case Grindstone::GraphicsAPI::API::Vulkan:
			return new ImguiRendererVulkan();
	}

	return nullptr;
}
