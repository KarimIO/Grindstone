#pragma once

#include "rapidjson/document.h"
#include "ShaderReflectionData.hpp"

namespace Grindstone {
	class ShaderReflectionLoader {
		public:
			ShaderReflectionLoader(const char* path, ShaderReflectionData& data);
		private:
			void Process();
			void ProcessMetadata();
			void ProcessUniformBuffers();
			void ProcessTextures();
		private:
			rapidjson::Document document;
			ShaderReflectionData& outData;
	};

	void LoadShaderReflection(const char* path, ShaderReflectionData& reflectionData);
}
