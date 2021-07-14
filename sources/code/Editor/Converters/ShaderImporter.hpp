#pragma once

#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

namespace Grindstone {
	namespace Converters {
		class ShaderImporter {
			public:
				enum class ShaderType {
					Vertex,
					Fragment,
					Geometry,
					Compute,
					TesselationControl,
					TesselationEvaluation
				};
				struct Texture {
					std::string name;
					size_t binding;
				};

				struct UniformBuffer {
					struct Member {
						std::string name;
						std::string type;
						size_t offset;
						size_t memberSize;
						Member() = default;
						Member(
							std::string name,
							std::string type,
							size_t offset,
							size_t memberSize
						) : name(name), type(type), offset(offset), memberSize(memberSize) {}
					};

					std::string name;
					size_t binding;
					size_t buffserSize;
					std::vector<ShaderType> shaderPasses;
					std::vector<Member> members;
					UniformBuffer() = default;
					UniformBuffer(
						std::string name,
						size_t binding,
						size_t buffserSize
					) : name(name), binding(binding), buffserSize(buffserSize) {}
				};
				ShaderImporter();
				void convertFile(const char* filePath);
			private:
				void process();
				void writeReflectionDocument();
				std::string extractField(const char* fieldKey);
				void extractName();
				void extractSubmodules();
				void processSubmodule(ShaderType shaderType, const char* extension, const char* glslSource);
				std::vector<uint32_t> convertToSpirv(ShaderType, const char* extension, const char* shaderModuleGlsl);
				std::string convertToOpenglGlsl(std::vector<uint32_t>&);
				void convertToOpenglSpirv(ShaderType, const char* extension, const char* openglGlsl);
				void reflectResources(ShaderType shaderType, std::vector<uint32_t>&);
				void outputStringToFile(const char* extension, const char* content);
				void outputUint32ToFile(const char* extension, std::vector<uint32_t>& content);
				ShaderType getShaderTypeFromString(std::string& str);
				const char* getShaderTypeExtension(ShaderType);
				const char* getShaderTypeAsString(ShaderType);
		private:
				std::string path;
				std::string basePath;
				std::string shaderName;
				std::string sourceFileContents;
				std::vector<ShaderType> shaderPasses;
				std::vector<Texture> textures;
				std::vector<UniformBuffer> uniformBuffers;
				rapidjson::StringBuffer reflectionStringBuffer;
				rapidjson::PrettyWriter<rapidjson::StringBuffer> reflectionWriter;
		};

		void ImportShadersFromGlsl(const char* filePath);
	}
}