#include "SystemRegistrar.hpp"
using namespace Grindstone::ECS;

SystemRegistrar::SystemRegistrar() {
}

void SystemRegistrar::RegisterSystem(const char *name, SystemFactory factory) {
	systemFactories.emplace(name, factory);
}

void SystemRegistrar::Update(EngineCore* engineCore, entt::registry& registry) {
	for each (auto systemFactory in systemFactories) {
		auto systemFn = systemFactory.second;
		systemFn(engineCore, registry);
	}
}

SystemRegistrar::~SystemRegistrar() {
}