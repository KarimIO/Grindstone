#ifndef _CUBEMAP_SYSTEM_H
#define _CUBEMAP_SYSTEM_H

#include "BaseSystem.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <string>
#include <vector>
#include "Texture.hpp"

#include "CubeInfo.hpp"

class TextureBindingLayout;

struct CubemapComponent : public Component {
	CubemapComponent(GameObjectHandle object_handle, ComponentHandle id);
	uint32_t resolution_;
	Texture *cubemap_;
	TextureBinding *cubemap_binding_;
	float near_;
	float far_;
	enum CaptureMethod {
		CAPTURE_REALTIME = 0,
		CAPTURE_BAKE,
		CAPTURE_CUSTOM
	} capture_method_;
};

class CubemapSystem : public System {
public:
	CubemapSystem();
	void update(double dt);
};

class CubemapSubSystem : public SubSystem {
	friend CubemapSystem;
public:
	CubemapSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params);
	CubemapComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);

	void captureCubemaps(double);
	void loadCubemaps();
	CubemapComponent *getClosestCubemap(glm::vec3);

	virtual ~CubemapSubSystem();
private:
	std::vector<CubemapComponent> components_;
	TextureSubBinding cube_binding_;
	TextureBindingLayout *texture_binding_layout_;
};

#endif