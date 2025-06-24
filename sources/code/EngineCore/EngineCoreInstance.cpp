#include "EngineCore.hpp"

static Grindstone::EngineCore* engineCoreInstance = nullptr;

void Grindstone::EngineCore::SetInstance(Grindstone::EngineCore& engineCore) {
	engineCoreInstance = &engineCore;
}

Grindstone::EngineCore& Grindstone::EngineCore::GetInstance() {
	return *engineCoreInstance;
}
