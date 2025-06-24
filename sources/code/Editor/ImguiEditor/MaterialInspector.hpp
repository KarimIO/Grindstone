#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <rapidjson/document.h>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Editor/AssetRegistry.hpp>

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;

			struct Sampler {
				std::string name;
				Uuid value;
				std::string valueName;
				bool isSet;

				Sampler(const char* samplerName) : name(samplerName), isSet(false) {}
			};

			struct MaterialParameter {
				std::string path;
			};

			class MaterialInspector {
			public:
				MaterialInspector(EngineCore* engineCore, ImguiEditor* imguiEditor);
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
				void ReloadAvailablePipelineSets();
				bool TryLoadShaderReflection(Uuid& shaderUuid);
				void LoadShaderUniformBuffers(rapidjson::Value& uniformBuffers);
				void LoadMaterialSamplers(rapidjson::Value& samplers);
				void RenderTextures();
				void RenderParameters();
				void OnSelectedTexture(Uuid uuid, std::string name);
				void RenderTexture(Sampler& sampler);
				void RenderParameter(MaterialParameter& parameter);
				void SaveMaterial();
				std::filesystem::path materialPath;
				std::string filename;
				std::string materialName;
				Uuid shaderUuid;
				std::string pipelineSetName;
				std::vector<Sampler> samplers;
				std::vector<MaterialParameter> parameters;
				std::vector<AssetRegistry::Entry> availablePipelineSets;
				EngineCore* engineCore;
				ImguiEditor* imguiEditor = nullptr;
				Sampler* selectedSampler = nullptr;
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
