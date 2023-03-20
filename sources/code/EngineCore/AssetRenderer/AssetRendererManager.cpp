#include "AssetRendererManager.hpp"
#include "EngineCore/Profiling.hpp"
using namespace Grindstone;

void AssetRendererManager::AddAssetRenderer(BaseAssetRenderer* assetRenderer) {
	assetRenderers.push_back(assetRenderer);

	for (const char* name : assetQueuesNames) {
		assetRenderer->AddQueue(name);
	}
}

void AssetRendererManager::AddQueue(const char* name) {
	assetQueuesNames.emplace_back(name);

	for (BaseAssetRenderer* assetRenderer : assetRenderers) {
		assetRenderer->AddQueue(name);
	}
}

void AssetRendererManager::RenderQueue(const char* name) {
	std::string profileScope = std::string("AssetRendererManager::RenderQueue(") + name + ")";
	GRIND_PROFILE_SCOPE(profileScope.c_str());
	for (BaseAssetRenderer* assetRenderer : assetRenderers) {
		assetRenderer->RenderQueue(name);
	}
}
