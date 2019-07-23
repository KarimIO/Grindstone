#ifndef _RENDER_SKELETAL_MESH_SYSTEM_H
#define _RENDER_SKELETAL_MESH_SYSTEM_H

#include <vector>
#include "BaseSystem.hpp"
#include "AssetManagers/AssetReferences.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../AssetCommon/SkeletonLoader.hpp"

struct Animation {
	struct Keyframe {
		glm::vec3 position_;
		glm::quat rotation_;
		glm::vec3 scale_;
		double time_;
	};

	struct Node {
		std::vector <Node *> children_;
		std::vector<Keyframe> keyframe_;
		unsigned int bone_id_;
	};

	Node *root_;
	double length_;
};

struct AnimationPlayer {
	Animation *animation_;
	double current_time_;
	bool loop_;
};

struct Skeleton {
	glm::mat4 global_inverse_;
	std::vector<GrindstoneAssetCommon::BoneInfo> bone_info_;
};

struct AnimationComponent : public Component {
	AnimationComponent(GameObjectHandle object_handle, ComponentHandle handle);
	AnimationComponent(GameObjectHandle object_handle, ComponentHandle handle, std::string animation_path, std::string skeleton_path);

	void update(double dt);

	std::string animation_path_;
	std::string skeleton_path_;

	void play();
	void pause();
private:
	std::string path_;
	std::vector<AnimationPlayer> animations_;
	std::vector<glm::mat4> bones_animated_;
	bool playing_;
	Skeleton *skeleton_;

	void readNodeHeirarchy(double time, Animation::Node *node, glm::mat4x4 parent_transform);
};

class AnimationSystem : public System {
public:
	AnimationSystem();

	void update(double dt);
};

class AnimationSubSystem : public SubSystem {
	friend AnimationSystem;
public:
	AnimationSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	virtual void setComponent(ComponentHandle component_handle, rapidjson::Value & params) override;
	AnimationComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);
	virtual ~AnimationSubSystem();
private:
	std::vector<AnimationComponent> components_;
};

#endif