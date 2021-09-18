#pragma once

#include <string>

namespace Grindstone {
	namespace Converters {
		namespace Materials {
			struct StandardMaterialCreateInfo {
				std::string materialName = "";
				std::string albedoPath = "";
				std::string specularPath = "";
				std::string normalPath = "";
				std::string roughnessPath = "";
				float albedoColor[4];
				float metalness;
				float roughness;
			};

			bool CreateStandardMaterial(StandardMaterialCreateInfo ci, std::string path);
			bool CreateCutoutMaterial(StandardMaterialCreateInfo ci, std::string path);
		};
	}
}
