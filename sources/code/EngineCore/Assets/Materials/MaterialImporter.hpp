#pragma once

#include <filesystem>
#include <string>
#include <map>

#include <rapidjson/document.h>

#include <Common/Graphics/DescriptorSet.hpp>
#include <EngineCore/Assets/AssetImporter.hpp>
#include <EngineCore/Assets/Shaders/ShaderReflectionData.hpp>
#include "MaterialAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class Texture;
	}

	class BaseAssetRenderer;

	class MaterialImporter : public AssetImporter {
	public:
		MaterialImporter();
		virtual void QueueReloadAsset(Uuid uuid) override;
		virtual void* ProcessLoadedFile(Uuid uuid) override;
		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) override;
	private:
		void SetupUniformBuffer(rapidjson::Document& document, ShaderReflectionData& reflectionData, std::vector<GraphicsAPI::DescriptorSet::Binding>& bindings, std::string name, MaterialAsset* materialAsset);
		void SetupSamplers(rapidjson::Document& document, ShaderReflectionData& reflectionData, std::vector<GraphicsAPI::DescriptorSet::Binding>& bindings);

		std::map<Uuid, MaterialAsset> materials;
		GraphicsAPI::Texture* missingTexture = nullptr;
	};
}
