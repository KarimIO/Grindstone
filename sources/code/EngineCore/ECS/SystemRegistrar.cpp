#include "SystemRegistrar.hpp"
using namespace Grindstone::ECS;

SystemRegistrar::SystemRegistrar() {
}

void SystemRegistrar::registerSystem(const char *name, SystemFactory factory) {
	systemFactories.emplace(name, factory);
}

void SystemRegistrar::update(entt::registry& registry) {
	for each (auto systemFactory in systemFactories) {
		auto systemFn = systemFactory.second;
		systemFn(registry);
	}
}

SystemRegistrar::~SystemRegistrar() {
}