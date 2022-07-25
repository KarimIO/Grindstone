#pragma once

#include <filesystem>
#include <string>
#include <map>

#include "EngineCore/Assets/AssetImporter.hpp"
#include "Material.hpp"

namespace Grindstone {
	class BaseAssetRenderer;

	class MaterialImporter : public AssetImporter {
		public:
			virtual void Load(Uuid& uuid) override;
			virtual void LazyLoad(Uuid& uuid) override;

			virtual Material& LoadMaterial(BaseAssetRenderer* assetRenderer, const char* path);
			virtual void ReloadMaterialIfLoaded(const char* path);
			virtual void RemoveRenderableFromMaterial(std::string uuid, ECS::Entity entity, void* renderable);
		private:
			bool TryGetMaterial(const char* path, Material*& material);
			Material& CreateMaterialFromFile(BaseAssetRenderer* assetRenderer, const char* path);
			void CreateMaterialFromData(
				std::filesystem::path relativePath,
				Material& material,
				BaseAssetRenderer* assetRenderer,
				const char* data
			);
		private:
			std::map<std::string, Material> materials;
	};
}
