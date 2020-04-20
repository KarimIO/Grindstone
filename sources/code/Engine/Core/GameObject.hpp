#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <vector>
#include <string>
#include <Engine/Systems/BaseSystem.hpp>


class GameObject {
public:
	GameObject(const GameObject &g);
	GameObject(GameObjectHandle id, std::string name, GameObjectHandle parent);
	bool operator== (GameObject &other);
	void setComponentHandle(ComponentType, ComponentHandle);
	ComponentHandle getComponentHandle(ComponentType);
	GameObjectHandle getID();
	GameObjectHandle getParentID();
	std::string getName();
	void setName(std::string str);
	void removeComponent(ComponentType);
	void removeAllComponents();
	~GameObject();
private:
	std::string name_;
	GameObjectHandle id_;
	GameObjectHandle parent_;
	ComponentHandle components_[NUM_COMPONENTS];
	std::vector<ComponentHandle> game_components_;
};

#endif