#pragma once

#include <string>
#include <vector>

namespace Grindstone {
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
				void setMaterialPath(const char* materialPath);
				void render();
			private:
				void tryLoadShaderReflection();
				void renderTextures();
				void renderParameters();
				void renderTexture(MaterialTexture& texture);
				void renderParameter(MaterialParameter& parameter);
				std::string materialPath;
				std::string materialName;
				std::string shaderRelativePath;
				std::vector<MaterialTexture> textures;
				std::vector<MaterialParameter> parameters;
			};
		}
	}
}
