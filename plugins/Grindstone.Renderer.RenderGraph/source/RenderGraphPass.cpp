#include <Grindstone.Renderer.RenderGraph/include/RenderGraphPass.hpp>

using namespace Grindstone::Renderer;


void RenderGraphPass::AddInputImage(HashedString name, AttachmentInfo attachmentInfo) {
	dependencyResourceNames.push_back(name);
}

void RenderGraphPass::AddInputOutputImage(HashedString inName, HashedString outName, AttachmentInfo attachmentInfo) {}
void RenderGraphPass::AddOutputImage(HashedString name, AttachmentInfo attachmentInfo) {}

void RenderGraphPass::AddInputBuffer(HashedString name, BufferInfo bufferInfo) {}
void RenderGraphPass::AddInputOutputBuffer(HashedString inName, HashedString outName, BufferInfo bufferInfo) {}
void RenderGraphPass::AddOutputBuffer(HashedString name, BufferInfo bufferInf) {}

void RenderGraphPass::RenderEnabled() {
	OnRenderEnabled(nullptr);
}

void RenderGraphPass::RenderDisabled() {
	OnRenderDisabled(nullptr);
}

// Set up the pass for rendering in the future
RenderGraphPass& RenderGraphPass::SetOnSetup(std::function<void* ()> fn) {
	OnSetup = fn;
	return *this;
}

// Set up the pass for rendering in the future
RenderGraphPass& RenderGraphPass::SetOnDestroy(std::function<void(void*)> fn) {
	OnDestroy = fn;
	return *this;
}

// The real rendering of a pass.
RenderGraphPass& RenderGraphPass::SetOnRenderEnabled(std::function<void(void*)> fn) {
	OnRenderEnabled = fn;
	return *this;
}

// For when rendering a system is disabled but resources still need to be cleared, etc.
RenderGraphPass& RenderGraphPass::SetOnRenderDisabledCallback(std::function<void(void*)> fn) {
	OnRenderDisabled = fn;
	return *this;
}

Grindstone::HashedString RenderGraphPass::GetName() const { return name; }
