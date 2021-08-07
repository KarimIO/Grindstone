#include "BaseAssetRenderer.hpp"
using namespace Grindstone;

void BaseAssetRenderer::AddQueue(const char* name) {
	// renderQueues[name] = renderQueueContainer;
}

void BaseAssetRenderer::RenderQueue(const char* name) {
	auto& renderQueueInMap = renderQueues.find(std::string(name));
	if (renderQueueInMap == renderQueues.end()) {
		throw std::runtime_error("RenderQueue not found.");
	}

	RenderQueue(renderQueueInMap->second);
}
