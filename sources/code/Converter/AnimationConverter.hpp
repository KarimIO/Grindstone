#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/gtx/quaternion.hpp>
#include "../AssetCommon/SkeletonLoader.hpp"

class aiNode;
class aiAnimation;
class aiNodeAnim;

class AnimationConverter {
public:
	struct Params {
		std::string path;
		std::string skeleton_path;
		std::string output_path;
	} params_;

	AnimationConverter(Params params);
	AnimationConverter(aiScene *scene, std::string skeleton_path);
private:
	glm::mat4 global_inverse_;

	struct Keyframe {
		glm::vec3 position_;
		glm::quat rotation_;
		glm::vec3 scale_;
		double time_;
	};

	struct Node {
		std::vector<Keyframe> keyframes_;
	};

	void getAnimationLength();
	void processFrames();
	void processScene();
	void traverseNode(const aiNode *node);
	const aiNodeAnim *findNodeAnim(const aiAnimation *animation, std::string bone_name);

	aiScene *scene_;
	std::vector<GrindstoneAssetCommon::BoneInfo> bone_info_;

	float animation_length_;
	std::vector<AnimationConverter::Node> nodes_;
};

void parseAnimationConverterParams(std::string params);