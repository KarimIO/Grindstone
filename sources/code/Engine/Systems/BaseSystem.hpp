#ifndef _SYSTEM_H
#define _SYSTEM_H

typedef unsigned char ComponentHandle;
typedef unsigned int GameObjectHandle;

enum ComponentType {
	COMPONENT_TRANSFORM = 0,
	COMPONENT_CONTROLLER,
	COMPONENT_CAMERA,
	COMPONENT_MODEL,
	COMPONENT_RENDER,
	COMPONENT_GEOMETRY,
	COMPONENT_LIGHT_POINT,
	COMPONENT_LIGHT_SPOT,
	COMPONENT_LIGHT_DIRECTIONAL,
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

struct Component {
	ComponentType component_type_ = COMPONENT_BASE;
	GameObjectHandle game_object_id_;
	ComponentHandle id_;
};

class System {
public:
	virtual void update(double dt) = 0;
	virtual Component *addComponent() = 0;
	virtual void removeComponent(ComponentHandle id) = 0;
	virtual ~System() = 0;

	ComponentType system_type_;
};

#endif