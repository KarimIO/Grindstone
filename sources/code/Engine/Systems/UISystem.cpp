#include "Core/Engine.hpp"
#include "UiSystem.hpp"

UiComponent::UiComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_UI, object_handle, handle) {}

UiSubSystem::UiSubSystem(Space *space) : SubSystem(COMPONENT_UI, space) {
}

ComponentHandle UiSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	return component_handle;
}

UiSubSystem::~UiSubSystem() {
}

void UiSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

UiComponent &UiSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

Component * UiSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t UiSubSystem::getNumComponents() {
	return components_.size();
}

void UiSystem::update() {
}

UiSystem::UiSystem() : System(COMPONENT_UI) {}

REFLECT_STRUCT_BEGIN(UiComponent, UiSystem, COMPONENT_UI)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
