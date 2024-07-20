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
	Grindstone::Memory::AllocatorCore& allocator = engineCore.GetAllocator();

	switch (graphicsCore->GetAPI()) {
		case Grindstone::GraphicsAPI::API::OpenGL:
			throw new std::runtime_error("OpenGL is currently not supported.");
		case Grindstone::GraphicsAPI::API::Vulkan:
			return allocator.Allocate<ImguiRendererVulkan>();
	}

	return nullptr;
}
