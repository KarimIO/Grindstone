#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <vector>
#include "../Systems/BaseSystem.hpp"


class GameObject {
public:
	GameObject(GameObjectHandle id, std::string name);
	bool operator== (GameObject &other);
	void setComponentHandle(ComponentType, ComponentHandle);
	ComponentHandle getComponentHandle(ComponentType);
	GameObjectHandle getID();
	std::string getName();
	~GameObject();
private:
	std::string name_;
	GameObjectHandle id_;
	ComponentHandle components_[NUM_COMPONENTS];
};

#endif