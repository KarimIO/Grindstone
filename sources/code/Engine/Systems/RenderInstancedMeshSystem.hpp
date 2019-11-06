#ifndef _RENDER_INSTANCED_MESH_SYSTEM_H
#define _RENDER_INSTANCED_MESH_SYSTEM_H

#include <vector>
#include "BaseSystem.hpp"
#include "AssetManagers/AssetReferences.hpp"

struct RenderInstancedMeshComponent : public Component {
	RenderInstancedMeshComponent(GameObjectHandle object_handle, ComponentHandle handle);
	std::string path_;
	ModelReference model_handle_;
};

class RenderInstancedMeshSystem : public System {
public:
	RenderInstancedMeshSystem();

	void update();
};

class RenderInstancedMeshSubSystem : public SubSystem {
	friend RenderInstancedMeshSystem;
public:
	RenderInstancedMeshSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	RenderInstancedMeshComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
	virtual ~RenderInstancedMeshSubSystem();
private:
	std::vector<RenderInstancedMeshComponent> components_;
};

#endif