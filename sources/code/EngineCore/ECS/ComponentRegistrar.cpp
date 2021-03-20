#include "ComponentRegistrar.hpp"
using namespace Grindstone::ECS;

void ComponentRegistrar::registerComponent(const char *name, ComponentFunctions componentFunctions) {
	componentFunctionsList.emplace(name, componentFunctions);
}

void* ComponentRegistrar::createComponent(const char *name, entt::registry& registry, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	return selectedFactory->second.createComponentFn(registry, entity);
}

bool ComponentRegistrar::tryGetComponent(const char *name, entt::registry& registry, ECS::Entity entity, void*& outComponent) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	outComponent = selectedFactory->second.createComponentFn(registry, entity);
	return true;
}

bool ComponentRegistrar::tryGetComponentReflectionData(const char *name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	outReflectionData = selectedFactory->second.getComponentReflectionDataFn();
	return true;
}

ComponentRegistrar::ComponentMap::iterator ComponentRegistrar::begin() {
	return componentFunctionsList.begin();
}

ComponentRegistrar::ComponentMap::const_iterator ComponentRegistrar::begin() const {
	return componentFunctionsList.begin();
}

ComponentRegistrar::ComponentMap::iterator ComponentRegistrar::end() {
	return componentFunctionsList.end();
}

ComponentRegistrar::ComponentMap::const_iterator ComponentRegistrar::end() const {
	return componentFunctionsList.end();
}

