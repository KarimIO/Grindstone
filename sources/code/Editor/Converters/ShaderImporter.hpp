#pragma once

#include <string>
#include <vector>

namespace Grindstone {
	namespace Converters {
		class ShaderImporter {
			public:
				void convertFile(const char* filePath);
			private:
				void process();
				void convertToSpirv();
				std::string convertToOpenglGlsl();
				void convertToOpenglSpirv(std::string& glsl);
				void reflectResources();
				void outputStringToFile(const char* extension, std::string& content);
				void outputUint32ToFile(const char* extension, std::vector<uint32_t>& content);
				std::string path;
				std::string basePath;
				std::string vkGlslSource;
				std::vector<uint32_t> vkSpirv;
		};
	}
}