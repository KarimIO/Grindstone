#include <EngineCore/Profiling.hpp>
#include <EngineCore/Logger.hpp>

#include "SystemRegistrar.hpp"

using namespace Grindstone::ECS;

SystemRegistrar::SystemRegistrar() {
}

void SystemRegistrar::RegisterSystem(const char* name, SystemFactory factory) {
	auto sys = editorSystemFactories.find(name);
	if (sys != editorSystemFactories.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Registering a system that has already been registered: {}", name);
		return;
	}

	systemFactories.emplace(name, factory);
}

void SystemRegistrar::RegisterEditorSystem(const char* name, SystemFactory factory) {
	auto sys = editorSystemFactories.find(name);
	if (sys != editorSystemFactories.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Registering an editor system that has already been registered: {}", name);
		return;
	}

	editorSystemFactories.emplace(name, factory);
}

void SystemRegistrar::UnregisterSystem(const char* name) {
	auto sys = systemFactories.find(name);
	if (sys == systemFactories.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Unregistering a system that isn't registered: {}", name);
		return;
	}

	systemFactories.erase(sys);
}

void SystemRegistrar::UnregisterEditorSystem(const char* name) {
	auto sys = editorSystemFactories.find(name);
	if (sys == editorSystemFactories.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Unregistering an editor system that isn't registered: {}", name);
		return;
	}

	editorSystemFactories.erase(sys);
}

void SystemRegistrar::Update(entt::registry& registry) {
	GRIND_PROFILE_SCOPE("SystemRegistrar::Update");
	for (auto& systemFactory : systemFactories) {
		GRIND_PROFILE_SCOPE(systemFactory.first.c_str());
		auto systemFn = systemFactory.second;
		systemFn(registry);
	}
}

void SystemRegistrar::EditorUpdate(entt::registry& registry) {
	GRIND_PROFILE_SCOPE("SystemRegistrar::EditorUpdate");
	for (auto& systemFactory : editorSystemFactories) {
		GRIND_PROFILE_SCOPE(systemFactory.first.c_str());
		auto systemFn = systemFactory.second;
		systemFn(registry);
	}
}

SystemRegistrar::~SystemRegistrar() {
	systemFactories.clear();
	editorSystemFactories.clear();
}
