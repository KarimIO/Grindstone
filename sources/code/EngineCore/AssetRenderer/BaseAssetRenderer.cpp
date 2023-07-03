#include "BaseAssetRenderer.hpp"
using namespace Grindstone;

void BaseAssetRenderer::AddQueue(const char* name) {
	renderQueues[name] = RenderQueueContainer{};
}

void BaseAssetRenderer::RenderQueue(GraphicsAPI::CommandBuffer* commandBuffer, const char* name) {
	auto& renderQueueInMap = renderQueues.find(std::string(name));
	if (renderQueueInMap == renderQueues.end()) {
		throw std::runtime_error("RenderQueue not found.");
	}

	RenderQueue(commandBuffer, renderQueueInMap->second);
}

void BaseAssetRenderer::RenderQueueImmediate(const char* name) {
	auto& renderQueueInMap = renderQueues.find(std::string(name));
	if (renderQueueInMap == renderQueues.end()) {
		throw std::runtime_error("RenderQueue not found.");
	}

	RenderQueueImmediate(renderQueueInMap->second);
}
