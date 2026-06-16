#include <EngineCore/ECS/ComponentRegistrar.hpp>

#include "WorldContextSet.hpp"

Grindstone::WorldContextSet::WorldContextSet(const std::string& name) : name(name), registry(), contexts() {
}

void Grindstone::WorldContextSet::Reset() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::ECS::ComponentRegistrar* compReg = engineCore.GetComponentRegistrar();
	compReg->CallDestroyOnRegistry(*this);
	// TODO: Why does this cause a crash? We should have it to catch any potential issues, but then again
	// registry's destructor should catch the rest.
	// registry.clear();
}

Grindstone::WorldContextSet::~WorldContextSet() {
	Reset();
}
