#ifndef _RENDER_SKELETAL_MESH_SYSTEM_H
#define _RENDER_SKELETAL_MESH_SYSTEM_H

#include <vector>
#include "BaseSystem.hpp"
#include "AssetManagers/AssetReferences.hpp"

struct RenderSkeletalMeshComponent : public Component {
	RenderSkeletalMeshComponent(GameObjectHandle object_handle, ComponentHandle handle);
	std::string path_;
	ModelReference model_handle_;
};

class RenderSkeletalMeshSystem : public System {
public:
	RenderSkeletalMeshSystem();

	void update(double dt);
};

class RenderSkeletalMeshSubSystem : public SubSystem {
	friend RenderSkeletalMeshSystem;
public:
	RenderSkeletalMeshSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params);
	RenderSkeletalMeshComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
	virtual ~RenderSkeletalMeshSubSystem();
private:
	std::vector<RenderSkeletalMeshComponent> components_;
};

#endif