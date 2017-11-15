#include "SGameplay.hpp"

void SGameplay::AddComponent(CGameplay * component) {
}

void SGameplay::RemoveComponent(CGameplay * component) {
}

void SGameplay::AddSystem(SGBase *system) {
	subsystems.insert(system);
}

void SGameplay::RemoveSystem(SGBase *system) {
	subsystems.erase(system);
}

void SGameplay::Update(double dt) {
	for (auto &system : subsystems) {
		system->Update(dt);
	}
}

SGameplay::~SGameplay() {
	for (auto &system : subsystems) {
		delete system;
	}
}