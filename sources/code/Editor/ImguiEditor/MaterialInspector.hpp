#pragma once

#include <string>
#include <vector>
#include "rapidjson/document.h"

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			struct MaterialTexture {
				std::string path;
			};

			struct MaterialParameter {
				std::string path;
			};

			class MaterialInspector {
			public:
				MaterialInspector(EngineCore* engineCore);
				void setMaterialPath(const char* materialPath);
				void render();
			private:
				void tryLoadShaderReflection();
				void loadShaderUniformBuffers(rapidjson::Document& document);
				void renderTextures();
				void renderParameters();
				void renderTexture(MaterialTexture& texture);
				void renderParameter(MaterialParameter& parameter);
				std::string materialPath;
				std::string materialName;
				std::string shaderPath;
				std::string shaderName;
				std::vector<MaterialTexture> textures;
				std::vector<MaterialParameter> parameters;
				EngineCore* engineCore;
				bool hasLoadFile = true;

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
