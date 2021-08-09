#include "ComponentRegistrar.hpp"
using namespace Grindstone::ECS;

void ComponentRegistrar::RegisterComponent(const char *name, ComponentFunctions componentFunctions) {
	componentFunctionsList.emplace(name, componentFunctions);
}

void* ComponentRegistrar::CreateComponent(const char *name, entt::registry& registry, ECS::EntityHandle entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	return selectedFactory->second.CreateComponentFn(registry, entity);
}

void ComponentRegistrar::RemoveComponent(const char *name, entt::registry& registry, ECS::EntityHandle entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	return selectedFactory->second.RemoveComponentFn(registry, entity);
}

bool ComponentRegistrar::HasComponent(const char* name, entt::registry& registry, ECS::EntityHandle entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	return selectedFactory->second.HasComponentFn(registry, entity);
}

bool ComponentRegistrar::TryGetComponent(const char* name, entt::registry& registry, ECS::EntityHandle entity, void*& outComponent) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	return selectedFactory->second.TryGetComponentFn(registry, entity, outComponent);
}

bool ComponentRegistrar::TryGetComponentReflectionData(const char *name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	outReflectionData = selectedFactory->second.GetComponentReflectionDataFn();
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
