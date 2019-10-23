#ifndef _RENDER_STATIC_MESH_SYSTEM_H
#define _RENDER_STATIC_MESH_SYSTEM_H

#include <vector>
#include "BaseSystem.hpp"
#include "AssetManagers/AssetReferences.hpp"

struct RenderStaticMeshComponent : public Component {
	RenderStaticMeshComponent(GameObjectHandle object_handle, ComponentHandle handle);
	std::string path_;
	ModelReference model_handle_;

	REFLECT()
};

class RenderStaticMeshSystem : public System {
public:
	RenderStaticMeshSystem();

	void update(double dt);

	REFLECT_SYSTEM()
};

class RenderStaticMeshSubSystem : public SubSystem {
	friend RenderStaticMeshSystem;
public:
	RenderStaticMeshSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	RenderStaticMeshComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	void renderDebug(glm::mat4 &matrix);
	virtual void removeComponent(ComponentHandle handle);
	virtual ~RenderStaticMeshSubSystem();
	virtual void initialize() override;
private:
	std::vector<RenderStaticMeshComponent> components_;
};

#endif