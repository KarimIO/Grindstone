#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "rapidjson/document.h"
#include "./AssetManagers/AssetReferences.hpp"
#include <string>

enum ComponentType {
	COMPONENT_TRANSFORM = 0,
	COMPONENT_CONTROLLER,
	COMPONENT_CAMERA,
	COMPONENT_RENDER_STATIC_MESH,
	COMPONENT_RENDER_TERRAIN,
	COMPONENT_LIGHT,
	COMPONENT_PHYSICS,
	COMPONENT_INPUT,
	COMPONENT_AUDIO_SOURCE,
	COMPONENT_AUDIO_LISTENER,
	COMPONENT_SCRIPT,
	COMPONENT_GAME_LOGIC,
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
	SubSystem(ComponentType type);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) = 0;
	virtual void removeComponent(ComponentHandle id) = 0;

	ComponentType getSystemType();
	virtual ~SubSystem();
private:
	ComponentType system_type_;
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