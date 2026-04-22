#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>

#include <Common/Window/WindowManager.hpp>

// ===============================================================
// - RenderPassExecution
// ===============================================================

Grindstone::Renderer::RenderGraph::RenderGraph(std::vector<Grindstone::UniquePtr<Grindstone::Renderer::RenderGraphPass>>&& passes) : passes(std::move(passes)) {}

void Grindstone::Renderer::RenderGraph::ExecuteGraph(Grindstone::Renderer::RenderGraphContext context) {
	for (auto& pass : passes) {
		pass->Execute(context);
	}
}
