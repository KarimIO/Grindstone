#include "ComponentRegistrar.hpp"
using namespace Grindstone::ECS;

ComponentRegistrar::ComponentRegistrar() {
}

void ComponentRegistrar::registerComponent(const char *name, ComponentFactory factory) {
	componentFactories.emplace(name, factory);
}

void* ComponentRegistrar::createComponent(const char *name, entt::registry& registry, ECS::Entity entity) {
	auto selectedFactory = componentFactories.find(name);
	if (selectedFactory == componentFactories.end()) {
		return nullptr;
	}

	return selectedFactory->second(registry, entity);
}

ComponentRegistrar::~ComponentRegistrar() {
}