#ifndef _RENDER_STATIC_MESH_SYSTEM_H
#define _RENDER_STATIC_MESH_SYSTEM_H

#include <vector>
#include "BaseSystem.hpp"
#include "AssetManagers/AssetReferences.hpp"

struct RenderStaticMeshComponent : public Component {
	RenderStaticMeshComponent(GameObjectHandle object_handle, ComponentHandle handle);
	std::string path_;
	ModelReference model_handle_;
};

class RenderStaticMeshSystem : public System {
public:
	RenderStaticMeshSystem();

	void update(double dt);
};

class RenderStaticMeshSubSystem : public SubSystem {
	friend RenderStaticMeshSystem;
public:
	RenderStaticMeshSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	RenderStaticMeshComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
	virtual ~RenderStaticMeshSubSystem();
private:
	std::vector<RenderStaticMeshComponent> components_;
};

#endif