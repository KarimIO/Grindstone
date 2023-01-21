#include "BaseAssetRenderer.hpp"
using namespace Grindstone;

void BaseAssetRenderer::AddShaderToRenderQueue(ShaderAsset* shader) {
	// const char* renderQueue = shader->reflectionData.renderQueue.c_str();
	// renderQueues[renderQueue].shaders.push_back(shader);
}

void BaseAssetRenderer::AddQueue(const char* name) {
	renderQueues[name] = RenderQueueContainer{};
}

void BaseAssetRenderer::RenderQueue(const char* name) {
	auto& renderQueueInMap = renderQueues.find(std::string(name));
	if (renderQueueInMap == renderQueues.end()) {
		throw std::runtime_error("RenderQueue not found.");
	}

	RenderQueue(renderQueueInMap->second);
}
