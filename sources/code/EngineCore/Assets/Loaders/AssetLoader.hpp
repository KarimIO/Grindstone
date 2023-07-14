#pragma once

#include <filesystem>
#include <vector>

#include <Common/Graphics/GraphicsPipeline.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	namespace Assets {
		class AssetLoader {
		public:
			virtual void Load(std::filesystem::path path, char*& outContents, size_t& fileSize) = 0;
			virtual void Load(Uuid uuid, char*& outContents, size_t& fileSize) = 0;
			virtual bool LoadText(Uuid uuid, std::string& outContents) = 0;
			virtual bool LoadShaderStage(
				Uuid uuid,
				GraphicsAPI::ShaderStage shaderStage,
				GraphicsAPI::ShaderStageCreateInfo& shaderStageCreateInfo,
				std::vector<char>& fileData
			) = 0;
		};
	}
}
