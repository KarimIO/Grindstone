#include "AssetRendererManager.hpp"
using namespace Grindstone;

void AssetRendererManager::AddAssetRenderer(BaseAssetRenderer* assetRenderer) {
	assetRenderers.push_back(assetRenderer);
}

void AssetRendererManager::AddQueue(const char* name) {
	for (BaseAssetRenderer* assetRenderer : assetRenderers) {
		assetRenderer->AddQueue(name);
	}
}

void AssetRendererManager::RenderQueue(const char* name) {
	for (BaseAssetRenderer* assetRenderer : assetRenderers) {
		assetRenderer->RenderQueue(name);
	}
}
