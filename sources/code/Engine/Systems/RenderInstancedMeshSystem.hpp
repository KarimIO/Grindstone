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

	void update(double dt);
};

class RenderInstancedMeshSubSystem : public SubSystem {
	friend RenderInstancedMeshSystem;
public:
	RenderInstancedMeshSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	virtual void setComponent(ComponentHandle component_handle, rapidjson::Value & params) override;
	RenderInstancedMeshComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);
	virtual ~RenderInstancedMeshSubSystem();
private:
	std::vector<RenderInstancedMeshComponent> components_;
};

#endif