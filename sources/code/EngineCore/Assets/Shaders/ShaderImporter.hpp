#pragma once

#include <string>
#include <map>
#include <fstream>

#include "EngineCore/Assets/AssetImporter.hpp"
#include "Shader.hpp"

namespace Grindstone {
	class BaseAssetRenderer;
	class ShaderImporter : public AssetImporter {
		public:
			virtual void Load(Uuid& uuid) override;
			virtual void LazyLoad(Uuid& uuid) override;
			virtual Shader& LoadShader(BaseAssetRenderer* assetRenderer, const char* path);
			virtual void ReloadShaderIfLoaded(const char* path);
			virtual void RemoveMaterialFromShader(Shader* shader, Material* material);
		private:
			bool TryGetShader(const char* path, Shader*& shader);
			void LoadShaderFromFile(bool isReloading, std::string& path, Shader& shaderAsset);
			Shader& CreateNewShaderFromFile(std::string& path);
			void CreateReflectionDataForShader(const char* path, Shader& shader);
			void CreateShaderGraphicsPipeline(bool isReloading, const char* path, Shader& shader);
		private:
			std::map<std::string, Shader> shaders;
	};
}