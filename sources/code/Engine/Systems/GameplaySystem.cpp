#if 0
#include "GameplaySystem.hpp"

void GameplaySystem::AddComponent(GameplayComponentComponent * component) {
}

void GameplaySystem::RemoveComponent(CGameplayComponent * component) {
}

void GameplaySystem::AddSystem(SGBase *system) {
	subsystems.insert(system);
}

void GameplaySystem::RemoveSystem(SGBase *system) {
	subsystems.erase(system);
}

void GameplaySystem::update() {
	GRIND_PROFILE_FUNC();
	for (auto &system : subsystems) {
		system->Update(dt);
	}
}

GameplaySystem::~GameplaySystem() {
	for (auto &system : subsystems) {
		delete system;
	}
}
#endif