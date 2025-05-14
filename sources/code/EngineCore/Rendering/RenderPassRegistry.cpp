#include "RenderPassRegistry.hpp"

void Grindstone::RenderPassRegistry::RegisterRenderpass(Grindstone::HashedString key, Grindstone::GraphicsAPI::RenderPass* renderPass) {
	registry[key] = renderPass;
}

Grindstone::GraphicsAPI::RenderPass* Grindstone::RenderPassRegistry::GetRenderpass(Grindstone::HashedString key) {
	auto& it = registry.find(key);
	if (it == registry.end()) {
		return nullptr;
	}

	return it->second;
}

void Grindstone::RenderPassRegistry::UnregisterRenderpass(Grindstone::HashedString key) {
	registry.erase(key);
}
