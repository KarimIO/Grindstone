#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "LightPointSystem.hpp"

LightPointComponent::LightPointComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_LIGHT_POINT, object_handle, id) {}

LightPointSubSystem::LightPointSubSystem(Space *space) : SubSystem(COMPONENT_LIGHT_POINT, space) {}

ComponentHandle LightPointSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

LightPointSystem::LightPointSystem() : System(COMPONENT_LIGHT_POINT) {}

void LightPointSystem::update() {
	GRIND_PROFILE_FUNC();
	for (auto space : engine.getSpaces()) {
		LightPointSubSystem *subsystem = (LightPointSubSystem *)space->getSubsystem(system_type_);
		for (auto &component : subsystem->components_) {
			// Culling

			// CalculateView

			// Render
		}
	}
}

void LightPointSystem::loadGraphics()
{
}

void LightPointSystem::destroyGraphics()
{
}

void LightPointSubSystem::setShadow(ComponentHandle h, bool shadow) {
	auto &component = components_[h];

	if (shadow) {
		auto graphics_wrapper = engine.getGraphicsWrapper();
	}
}

void LightPointSubSystem::initialize() {
	for (auto &c : components_) {
		setShadow(c.handle_, c.properties_.shadow);
	}
}

LightPointComponent & LightPointSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * LightPointSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t LightPointSubSystem::getNumComponents() {
	return components_.size();
}

void LightPointSubSystem::removeComponent(ComponentHandle handle) {
	GameObjectHandle h = components_[handle].game_object_handle_;
	space_->getObject(h).setComponentHandle(system_type_, UINT_MAX);
	
	components_.erase(components_.begin() + handle);

	for (ComponentHandle h = handle; h < components_.size(); ++h) {
		auto comp = components_[h];
		comp.handle_ = h;
		space_->getObject(comp.game_object_handle_).setComponentHandle(system_type_, h);
	}
}

LightPointSubSystem::~LightPointSubSystem() {
}

REFLECT_STRUCT_BEGIN(LightPointComponent, LightPointSystem, COMPONENT_LIGHT_POINT)
	REFLECT_STRUCT_MEMBER(properties_.color)
	REFLECT_STRUCT_MEMBER(properties_.power)
	REFLECT_STRUCT_MEMBER(properties_.attenuationRadius)
	REFLECT_STRUCT_MEMBER_D(properties_.shadow, "Enable Shadows", "castshadow", reflect::Metadata::ViewInAll, nullptr)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
