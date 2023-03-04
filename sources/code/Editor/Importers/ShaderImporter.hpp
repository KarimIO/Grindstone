#pragma once

#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "Importer.hpp"

namespace Grindstone {
	namespace Importers {
		class ShaderImporter : public Importer {
			public:
				void Import(std::filesystem::path& path) override;
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
					std::vector<ShaderType> shaderPasses;
					Texture() = default;
					Texture(
						std::string name,
						size_t binding
					) : name(name), binding(binding) {}
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
					size_t bufferSize;
					std::vector<ShaderType> shaderPasses;
					std::vector<Member> members;
					UniformBuffer() = default;
					UniformBuffer(
						std::string name,
						size_t binding,
						size_t bufferSize
					) : name(name), binding(binding), bufferSize(bufferSize) {}
				};
			private:
				void Process();
				void WriteReflectionStruct(std::vector<UniformBuffer>& structs);
				void WriteReflectionImage(std::vector<Texture>& resources);
				void WriteReflectionDocument();
				std::string ExtractField(const char* fieldKey);
				void ExtractSubmodules();
				void ProcessSubmodule(ShaderType shaderType, const char* extension, const char* glslSource);
				std::vector<uint32_t> ConvertToSpirv(ShaderType, const char* extension, const char* shaderModuleGlsl);
				std::string ConvertToOpenglGlsl(const char* extension, std::vector<uint32_t>&);
				void ConvertToOpenglSpirv(ShaderType, const char* extension, const char* openglGlsl);
				void ReflectResources(ShaderType shaderType, std::vector<uint32_t>&);
				void OutputStringToFile(const char* extension, const char* content);
				void OutputUint32ToFile(const char* extension, std::vector<uint32_t>& content);
				ShaderType GetShaderTypeFromString(std::string& str);
				const char* GetShaderTypeExtension(ShaderType);
				const char* GetShaderTypeAsString(ShaderType);
		private:
				std::filesystem::path inputPath;
				std::string basePath;
				std::string shaderName;
				std::string renderQueue;
				std::string sourceFileContents;
				std::vector<ShaderType> shaderPasses;
				std::vector<Texture> textures;
				std::vector<Texture> samplers;
				std::vector<UniformBuffer> uniformBuffers;
				rapidjson::StringBuffer reflectionStringBuffer;
				rapidjson::PrettyWriter<rapidjson::StringBuffer> reflectionWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>(reflectionStringBuffer);
		};

		void ImportShadersFromGlsl(std::filesystem::path& path);
	}
}
