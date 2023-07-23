#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "rapidjson/document.h"
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			struct Sampler {
				std::string name;
				Uuid value;
				bool isSet;

				Sampler(const char* samplerName) : name(samplerName), isSet(false) {}
			};

			struct MaterialParameter {
				std::string path;
			};

			class MaterialInspector {
			public:
				MaterialInspector(EngineCore* engineCore);
				void SetMaterialPath(const std::filesystem::path& materialPath);
				void Render();
			private:
				enum class ShaderLoadStatus {
					Unassigned = 0,
					InvalidPath,
					NoFileFound,
					InvalidShader,
					ValidShader
				};
				bool TryLoadShaderReflection(Uuid& shaderUuid);
				void LoadShaderTextures(rapidjson::Value& texturesArray);
				void LoadShaderUniformBuffers(rapidjson::Value& uniformBuffers);
				void LoadMaterialSamplers(rapidjson::Value& samplers);
				void RenderTextures();
				void RenderParameters();
				void RenderTexture(Sampler& sampler);
				void RenderParameter(MaterialParameter& parameter);
				void SaveMaterial();
				std::filesystem::path materialPath;
				std::string filename;
				std::string materialName;
				Uuid shaderUuid;
				std::string shaderName;
				std::vector<Sampler> samplers;
				std::vector<MaterialParameter> parameters;
				EngineCore* engineCore;
				ShaderLoadStatus shaderLoadStatus = ShaderLoadStatus::Unassigned;
				bool hasBeenChanged = false;

				struct MaterialUniformBuffer {
					struct Member {
						std::string name;
						size_t offset = 0;
						size_t memberSize = 0;
						Member() = default;
						Member(
							std::string name,
							size_t offset,
							size_t memberSize
						) : name(name), offset(offset), memberSize(memberSize) {}
					};
					std::string name;
					size_t bindingId = 0;
					size_t bufferSize = 0;
					std::vector<Member> members;
					MaterialUniformBuffer() = default;
					MaterialUniformBuffer(
						std::string name,
						size_t bindingId,
						size_t bufferSize
					) : name(name), bindingId(bindingId), bufferSize(bufferSize) {}
				};

				std::vector<MaterialUniformBuffer> materialUniformBuffers;
			};
		}
	}
}
