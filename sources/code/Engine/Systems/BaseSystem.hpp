#ifndef _SYSTEM_H
#define _SYSTEM_H

#undef Bool
#include "rapidjson/document.h"

#include "./AssetManagers/AssetReferences.hpp"
#include <string>

class Space;

static const char *component_names[] = {
	"COMPONENT_TRANSFORM",
	"COMPONENT_GAME_LOGIC",
	"COMPONENT_SCRIPT",
	"COMPONENT_CONTROLLER",
	"COMPONENT_INPUT",
	"COMPONENT_RIGID_BODY",
	"COMPONENT_COLLISION",
	"COMPONENT_ANIMATION",
	"COMPONENT_CUBEMAP",
	"COMPONENT_LIGHT_SPOT",
	"COMPONENT_LIGHT_POINT",
	"COMPONENT_LIGHT_DIRECTIONAL",
	"COMPONENT_CAMERA",
	"COMPONENT_RENDER_STATIC_MESH",
	"COMPONENT_RENDER_SKELETAL_MESH",
	"COMPONENT_RENDER_TERRAIN",
	"COMPONENT_AUDIO_SOURCE",
	"COMPONENT_AUDIO_LISTENER"
};

enum ComponentType {
	COMPONENT_TRANSFORM = 0,
	COMPONENT_GAME_LOGIC,
	COMPONENT_SCRIPT,
	COMPONENT_CONTROLLER,
	COMPONENT_INPUT,
	COMPONENT_RIGID_BODY,
	COMPONENT_COLLISION,
	COMPONENT_ANIMATION,
	COMPONENT_CUBEMAP,
	COMPONENT_LIGHT_SPOT,
	COMPONENT_LIGHT_POINT,
	COMPONENT_LIGHT_DIRECTIONAL,
	COMPONENT_CAMERA,
	COMPONENT_RENDER_STATIC_MESH,
	COMPONENT_RENDER_SKELETAL_MESH,
	COMPONENT_RENDER_TERRAIN,
	COMPONENT_AUDIO_SOURCE,
	COMPONENT_AUDIO_LISTENER,
	NUM_COMPONENTS,
	// Keeping this after, as it has to have an ID but shouldn't affect NUM_COMPONENTS
	COMPONENT_BASE
};

ComponentType getComponentType(std::string str);

struct Component {
	Component();
	Component(ComponentType, GameObjectHandle, ComponentHandle);
	ComponentType component_type_ = COMPONENT_BASE;
	GameObjectHandle game_object_handle_;
	ComponentHandle handle_;
};

class SubSystem {
public:
	SubSystem(ComponentType type, Space *space);
	virtual size_t getNumComponents() = 0;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) = 0;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) = 0;
	virtual void removeComponent(ComponentHandle id) = 0;

	ComponentType getSystemType();
	virtual ~SubSystem();
protected:
	ComponentType system_type_;
	Space *space_;
};

class Scene;

class System {
public:
	System(ComponentType type);
	virtual void update(double dt) = 0;
	virtual ~System();

	ComponentType system_type_;
};

#endif