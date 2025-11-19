#include <Grindstone.Renderer.RenderGraph/include/RenderGraph.hpp>

void Grindstone::Renderer::RenderGraph::Print() {
}

Grindstone::Renderer::RenderGraphPass& Grindstone::Renderer::RenderGraph::AddPass(RenderGraphPass pass) {
	return passes[pass.GetName()] = pass;
}

Grindstone::Renderer::RenderGraphPass& Grindstone::Renderer::RenderGraph::AddPass(HashedString name, GpuQueue queue) {
	return passes[name] = RenderGraphPass(name, queue);
}
