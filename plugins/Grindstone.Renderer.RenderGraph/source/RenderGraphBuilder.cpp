#include <Grindstone.Renderer.RenderGraph/include/RenderGraphBuilder.hpp>

using namespace Grindstone::Renderer;

RenderGraph Grindstone::Renderer::RenderGraphBuilder::Compile() {
	return RenderGraph();
}

RenderGraphPass& RenderGraphBuilder::AddPass(RenderGraphPass pass) {
	return passes[pass.GetName()] = pass;
}

RenderGraphPass& RenderGraphBuilder::AddPass(HashedString name, GpuQueue queue) {
	return passes[name] = RenderGraphPass(name, queue);
}

void RenderGraphBuilder::SetOutputAttachment(HashedString attachmentName) {
	outputAttachment = attachmentName;
}
