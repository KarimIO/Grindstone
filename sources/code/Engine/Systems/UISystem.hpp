#ifndef _UI_SYSTEM_HPP
#define _UI_SYSTEM_HPP

#include <vector>
#include "BaseSystem.hpp"

struct UiComponent : public Component {
	UiComponent(GameObjectHandle object_handle, ComponentHandle handle);

	REFLECT(COMPONENT_UI)
};

class UiSystem : public System {
public:
	UiSystem();

	void update();

	REFLECT_SYSTEM()
};

class UiSubSystem : public SubSystem {
	friend UiSystem;
public:
	UiSubSystem(Space *space);
	~UiSubSystem();
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	UiComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
private:
	std::vector<UiComponent> components_;
};

#endif