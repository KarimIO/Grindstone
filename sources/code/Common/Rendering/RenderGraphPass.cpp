#include <Common/Rendering/RenderGraphPass.hpp>

using namespace Grindstone::Renderer;


void RenderGraphPass::AddInputImage(HashedString name, AttachmentInfo attachmentInfo) {
	dependencyResourceNames.push_back(name);
}

void RenderGraphPass::AddInputOutputImage(HashedString inName, HashedString outName, AttachmentInfo attachmentInfo) {}
void RenderGraphPass::AddOutputImage(HashedString name, AttachmentInfo attachmentInfo) {}

void RenderGraphPass::AddInputBuffer(HashedString name, BufferInfo bufferInfo) {}
void RenderGraphPass::AddInputOutputBuffer(HashedString inName, HashedString outName, BufferInfo bufferInfo) {}
void RenderGraphPass::AddOutputBuffer(HashedString name, BufferInfo bufferInf) {}

Grindstone::HashedString RenderGraphPass::GetName() const { return name; }
