#include <EngineCore/ECS/ComponentRegistrar.hpp>

#include "WorldContextSet.hpp"

Grindstone::WorldContextSet::~WorldContextSet() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::ECS::ComponentRegistrar* compReg = engineCore.GetComponentRegistrar();
	compReg->CallDestroyOnRegistry(*this);
	registry.clear();
}
