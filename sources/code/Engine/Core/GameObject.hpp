#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <vector>
#include <string>
#include <Engine/Systems/BaseSystem.hpp>

class Space;

class GameObject {
public:
	// Regular Constructor
	GameObject(GameObjectHandle id, std::string name, Space *space, GameObjectHandle parent);
	// Copy Constructor
	GameObject(const GameObject &g);
	bool operator== (GameObject &other);
	template<typename ComponentT>
	ComponentT* createComponent();
	virtual Component *createComponent(ComponentType type);
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
	Space* space_;
	GameObjectHandle id_;
	GameObjectHandle parent_;
	ComponentHandle components_[NUM_COMPONENTS];
	std::vector<ComponentHandle> game_components_;
};

#endif