#include "BaseSystem.hpp"

ComponentType getComponentType(std::string str) {
	if (str == "COMPONENT_TRANSFORM")
		return COMPONENT_TRANSFORM;

	//if (str == "COMPONENT_CONTROLLER")
	//	return COMPONENT_CONTROLLER;

	if (str == "COMPONENT_CAMERA")
		return COMPONENT_CAMERA;
	
	if (str == "COMPONENT_RENDER_STATIC_MESH")
		return COMPONENT_RENDER_STATIC_MESH;
	
	/*if (str == "COMPONENT_RENDER_TERRAIN")
		return COMPONENT_RENDER_TERRAIN;
	
	if (str == "COMPONENT_LIGHT")
		return COMPONENT_LIGHT;
	
	if (str == "COMPONENT_PHYSICS")
		return COMPONENT_PHYSICS;
	
	if (str == "COMPONENT_INPUT")
		return COMPONENT_INPUT;
	
	if (str == "COMPONENT_AUDIO_SOURCE")
		return COMPONENT_AUDIO_SOURCE;
	
	if (str == "COMPONENT_AUDIO_LISTENER")
		return COMPONENT_AUDIO_LISTENER;
	
	if (str == "COMPONENT_SCRIPT")
		return COMPONENT_SCRIPT;
	
	if (str == "COMPONENT_GAME_LOGIC")
		return COMPONENT_GAME_LOGIC;*/

	return COMPONENT_BASE;
}

SubSystem::SubSystem(ComponentType type = COMPONENT_BASE) : system_type_(type) {
}

ComponentType SubSystem::getSystemType() {
	return system_type_;
}

Component::Component() {}

Component::Component(ComponentType type, GameObjectHandle game_object_handle, ComponentHandle component_handle) :
	component_type_(type),
	game_object_handle_(game_object_handle),
	handle_(component_handle) {}

SubSystem::~SubSystem() {}

System::System(ComponentType type) : system_type_(type) {}

System::~System() {}