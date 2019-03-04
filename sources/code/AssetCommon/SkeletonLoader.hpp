#ifndef _SKELETON_LOADER_HPP
#define _SKELETON_LOADER_HPP

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace GrindstoneAssetCommon {
	struct BoneInfo {
		std::string bone_name;
		glm::mat4 bone_offset;
	};

	void loadSkeleton(std::string path, glm::mat4 global_inverse, std::vector<BoneInfo> bone_info);
};

#endif