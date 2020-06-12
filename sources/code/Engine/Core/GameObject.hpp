#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <vector>
#include <string>
#include <Engine/Systems/BaseSystem.hpp>

class Space;

class GameObject {
public:
	// Regular Constructor
	GameObject(GameObjectHandle id, std::string name, Space *space);
	// Copy Constructor
	GameObject(const GameObject &g);
	bool operator== (GameObject &other);
	virtual Component *createComponent(ComponentType type);
	void setComponentHandle(ComponentType, ComponentHandle);
	ComponentHandle getComponentHandle(ComponentType);
	Component* getComponent(ComponentType);
	GameObjectHandle getID();
	std::string getName();
	void setName(std::string str);
	void removeComponent(ComponentType);
	void removeAllComponents();
	~GameObject();
public:
	template<typename ComponentT>
	ComponentT* createComponent() {
		ComponentType t = ComponentT::getComponentType();
		return (ComponentT *)createComponent(t);
	}
	template<typename ComponentT>
	ComponentT* getComponent() {
		ComponentType t = ComponentT::getComponentType();

		return (ComponentT *)getComponent(t);
	}
private:
	std::string name_;
	Space* space_;
	GameObjectHandle id_;
	ComponentHandle components_[NUM_COMPONENTS];
	std::vector<ComponentHandle> game_components_;
};

#endif