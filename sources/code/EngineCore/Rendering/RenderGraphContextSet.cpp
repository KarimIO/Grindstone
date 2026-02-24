#include <iostream>
#include <stdarg.h>

#include <EngineCore/Utils/MemoryAllocator.hpp>

#include <EngineCore/Rendering/RenderGraphContextSet.hpp>

using namespace Grindstone::Memory;

Grindstone::Rendering::RenderGraphWorldContext* activeContext = nullptr;

Grindstone::Rendering::RenderGraphWorldContext::RenderGraphWorldContext() {
}

Grindstone::Rendering::RenderGraphWorldContext* Grindstone::Rendering::RenderGraphWorldContext::GetActiveContext() {
	return activeContext;
}

void Grindstone::Rendering::RenderGraphWorldContext::SetActiveContext(RenderGraphWorldContext& cxt) {
	activeContext = &cxt;
}

void Grindstone::Rendering::RenderGraphWorldContext::SetAsActive() {
	activeContext = this;
}

void Grindstone::Rendering::RenderGraphWorldContext::StartFrame() {
}

Grindstone::Renderer::RenderGraph& Grindstone::Rendering::RenderGraphWorldContext::GetRenderGraph() {
	return renderGraph;
}
