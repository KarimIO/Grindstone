#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>

// ===============================================================
// - RenderPassExecution
// ===============================================================

void Grindstone::Renderer::RenderGraph::ExecuteGraph(Grindstone::Renderer::RenderGraphContext context) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::GraphicsAPI::CommandBuffer* cmd = context.commandBuffer;

}
