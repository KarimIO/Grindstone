#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "../Utilities/Reflection.hpp"

#undef Bool
#include <rapidjson/document.h>

#ifdef INCLUDE_EDITOR
#include <rapidjson/prettywriter.h>
#endif

#include "./AssetManagers/AssetReferences.hpp"
#include <string>

class Space;

#define FOREACH_SYSTEM(FORMAT) \
	FORMAT(COMPONENT_TRANSFORM) \
	FORMAT(COMPONENT_GAME_LOGIC) \
	FORMAT(COMPONENT_SCRIPT) \
	FORMAT(COMPONENT_CONTROLLER) \
	FORMAT(COMPONENT_INPUT) \
	FORMAT(COMPONENT_COLLISION) \
	FORMAT(COMPONENT_RIGID_BODY) \
	FORMAT(COMPONENT_ANIMATION) \
	FORMAT(COMPONENT_CUBEMAP) \
	FORMAT(COMPONENT_LIGHT_SPOT) \
	FORMAT(COMPONENT_LIGHT_POINT) \
	FORMAT(COMPONENT_LIGHT_DIRECTIONAL) \
	FORMAT(COMPONENT_CAMERA) \
	FORMAT(COMPONENT_RENDER_SPRITE) \
	FORMAT(COMPONENT_RENDER_STATIC_MESH) \
	FORMAT(COMPONENT_RENDER_INSTANCED_MESH) \
	FORMAT(COMPONENT_RENDER_SKELETAL_MESH) \
	FORMAT(COMPONENT_RENDER_TERRAIN) \
	FORMAT(COMPONENT_AUDIO_SOURCE) \
	FORMAT(COMPONENT_AUDIO_LISTENER)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

static const char* component_names[] = {
	FOREACH_SYSTEM(GENERATE_STRING)
};

enum ComponentType {
	FOREACH_SYSTEM(GENERATE_ENUM)
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
	virtual Component *getBaseComponent(ComponentHandle component_handle) = 0;
	virtual SubSystem *copy() { return nullptr; };
	virtual void initialize() {};

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
	virtual void update() = 0;
	virtual reflect::TypeDescriptor_Struct *getReflection() { return nullptr; };
	virtual ~System();

	ComponentType system_type_;
};

#endif