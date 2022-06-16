#include "ComponentRegistrar.hpp"
using namespace Grindstone::ECS;

void ComponentRegistrar::RegisterComponent(const char *name, ComponentFunctions componentFunctions) {
	componentFunctionsList.emplace(name, componentFunctions);
}

void* ComponentRegistrar::CreateComponentWithSetup(const char* name, ECS::Entity& entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	auto& fns = selectedFactory->second;
	auto comp = fns.CreateComponentFn(entity);

	if (fns.SetupComponentFn) {
		fns.SetupComponentFn(entity, comp);
	}

	return comp;
}

void* ComponentRegistrar::CreateComponent(const char* name, ECS::Entity& entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	return selectedFactory->second.CreateComponentFn(entity);
}

void ComponentRegistrar::RemoveComponent(const char *name, ECS::Entity& entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	return selectedFactory->second.RemoveComponentFn(entity);
}

bool ComponentRegistrar::HasComponent(const char* name, ECS::Entity& entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	return selectedFactory->second.HasComponentFn(entity);
}

bool ComponentRegistrar::TryGetComponent(const char* name, ECS::Entity& entity, void*& outComponent) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	return selectedFactory->second.TryGetComponentFn(entity, outComponent);
}

bool ComponentRegistrar::TryGetComponentReflectionData(const char *name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	outReflectionData = selectedFactory->second.GetComponentReflectionDataFn();
	return true;
}

void ComponentRegistrar::SetupComponent(const char* componentType, ECS::Entity& entity, void* componentPtr) {
	auto selectedFactory = componentFunctionsList.find(componentType);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	auto& fns = selectedFactory->second;
	if (fns.SetupComponentFn) {
		fns.SetupComponentFn(entity, componentPtr);
	}
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
