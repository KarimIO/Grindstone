#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/gtx/quaternion.hpp>

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

	struct Keyframe {
		glm::vec3 position_;
		glm::quat rotation_;
		glm::vec3 scale_;
		double time_;
	};

	struct Node {
		std::vector<Keyframe> keyframes_;
	};

	void parseSkeleton(std::string path);
	void getAnimationLength();
	void processFrames();
	void processScene();
	void traverseNode(const aiNode *node);
	const aiNodeAnim *findNodeAnim(const aiAnimation *animation, std::string bone_name);

	aiScene *scene_;
	std::vector<std::string> bone_map_;

	float animation_length_;
	std::vector<AnimationConverter::Node> nodes_;
};

void parseAnimationConverterParams(std::string params);