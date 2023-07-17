#pragma once

#include <filesystem>
#include <string>
#include <map>

#include "EngineCore/Assets/AssetImporter.hpp"
#include "MaterialAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class Texture;
	}

	class BaseAssetRenderer;

	class MaterialImporter : public AssetImporter {
	public:
		MaterialImporter();
		virtual void* ProcessLoadedFile(Uuid uuid) override;
		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) override;
	private:
		std::map<Uuid, MaterialAsset> materials;
		GraphicsAPI::Texture* missingTexture = nullptr;
	};
}
