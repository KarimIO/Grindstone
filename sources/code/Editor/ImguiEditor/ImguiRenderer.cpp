#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Common/Graphics/Core.hpp>
#include <Editor/EditorManager.hpp>

#include "ImguiRendererOpenGL.hpp"
#include "ImguiRendererVulkan.hpp"

#include "ImguiRenderer.hpp"

using namespace Grindstone::Editor::ImguiEditor;
using namespace Grindstone::Memory;

ImguiRenderer* ImguiRenderer::Create() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	switch (graphicsCore->GetAPI()) {
		case Grindstone::GraphicsAPI::API::OpenGL:
			throw new std::runtime_error("OpenGL is currently not supported.");
		case Grindstone::GraphicsAPI::API::Vulkan:
			return AllocatorCore::Allocate<ImguiRendererVulkan>();
	}

	return nullptr;
}
