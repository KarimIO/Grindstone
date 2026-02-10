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

void Grindstone::Rendering::RenderGraphWorldContext::JoinRenderGraphBuilder(Grindstone::Renderer::RenderGraphBuilder& otherBuilder) {
	renderGraphBuilder.JoinBuilder(otherBuilder);
}

Grindstone::Renderer::RenderGraphBuilder& Grindstone::Rendering::RenderGraphWorldContext::GetBuilder() {
	return renderGraphBuilder;
}
