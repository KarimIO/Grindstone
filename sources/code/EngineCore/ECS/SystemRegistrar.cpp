#include "SystemRegistrar.hpp"
using namespace Grindstone::ECS;

SystemRegistrar::SystemRegistrar() {
}

void SystemRegistrar::RegisterEditorSystem(const char *name, SystemFactory factory) {
	editorSystemFactories.emplace(name, factory);
}

void SystemRegistrar::RegisterSystem(const char* name, SystemFactory factory) {
	systemFactories.emplace(name, factory);
}

void SystemRegistrar::Update(entt::registry& registry) {
	for each (auto systemFactory in systemFactories) {
		auto systemFn = systemFactory.second;
		systemFn(registry);
	}
}

void SystemRegistrar::EditorUpdate(entt::registry& registry) {
	for each (auto systemFactory in editorSystemFactories) {
		auto systemFn = systemFactory.second;
		systemFn(registry);
	}
}

SystemRegistrar::~SystemRegistrar() {
}
