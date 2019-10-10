#ifndef _EDITOR_GAME_OBJECT_H
#define _EDITOR_GAME_OBJECT_H

#include <vector>
#include "../Systems/BaseSystem.hpp"

struct Prefab {
	std::vector<ComponentHandle> handle_;
};

class EditorGameObject {
public:
	GameObject(const GameObject &g);
	GameObject(GameObjectHandle id, std::string name);
	bool operator== (GameObject &other);
	void setComponentHandle(ComponentType, ComponentHandle);
	ComponentHandle getComponentHandle(ComponentType);
	GameObjectHandle getID();
	std::string getName();
	void setName(std::string str);
	void removeComponent(ComponentType);
	void removeAllComponents();
	~GameObject();
private:
	std::string name_;
	GameObjectHandle id_;
	ComponentHandle components_[NUM_COMPONENTS];
	std::vector<ComponentHandle> game_components_;
};

#endif