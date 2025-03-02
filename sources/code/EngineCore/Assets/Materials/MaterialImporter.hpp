#pragma once

#include <filesystem>
#include <string>
#include <map>

#include <rapidjson/document.h>

#include <Common/Graphics/DescriptorSet.hpp>
#include <EngineCore/Assets/AssetImporter.hpp>
#include "MaterialAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class Texture;
	}

	class BaseAssetRenderer;

	class MaterialImporter : public SpecificAssetImporter<MaterialAsset, AssetType::Material> {
	public:
		MaterialImporter();
		virtual ~MaterialImporter() override;
		virtual void QueueReloadAsset(Uuid uuid) override;
		virtual void* LoadAsset(Uuid uuid) override;

	private:
		GraphicsAPI::Texture* missingTexture = nullptr;
	};
}
