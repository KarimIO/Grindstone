#include "SystemRegistrar.hpp"
#include "EngineCore/Profiling.hpp"
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
	GRIND_PROFILE_FUNC();
	for (auto& systemFactory : systemFactories) {
		GRIND_PROFILE_SCOPE(systemFactory.first.c_str());
		auto systemFn = systemFactory.second;
		systemFn(registry);
	}
}

void SystemRegistrar::EditorUpdate(entt::registry& registry) {
	GRIND_PROFILE_FUNC();
	for (auto& systemFactory : editorSystemFactories) {
		GRIND_PROFILE_SCOPE(systemFactory.first.c_str());
		auto systemFn = systemFactory.second;
		systemFn(registry);
	}
}

SystemRegistrar::~SystemRegistrar() {
}
