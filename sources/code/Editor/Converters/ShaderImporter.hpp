#pragma once

#include <string>
#include <vector>

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

				void convertFile(const char* filePath);
			private:
				void process();
				std::string extractField(const char* fieldKey);
				void extractName();
				void extractSubmodules();
				void processSubmodule(ShaderType shaderType, const char* extension, const char* glslSource);
				std::vector<uint32_t> convertToSpirv(ShaderType, const char* extension, const char* shaderModuleGlsl);
				std::string convertToOpenglGlsl(std::vector<uint32_t>&);
				void convertToOpenglSpirv(ShaderType, const char* extension, const char* openglGlsl);
				void reflectResources(ShaderType shaderType, std::vector<uint32_t>&);
				void outputStringToFile(const char* extension, std::string& content);
				void outputUint32ToFile(const char* extension, std::vector<uint32_t>& content);
				std::string path;
				std::string basePath;
				std::string shaderName;
				std::string sourceFileContents;
				std::string reflectionFileOutput;
				ShaderType getShaderTypeFromString(std::string& str);
				const char* getShaderTypeExtension(ShaderType);
				const char* getShaderTypeAsString(ShaderType);
		};

		void ImportShadersFromGlsl(const char* filePath);
	}
}