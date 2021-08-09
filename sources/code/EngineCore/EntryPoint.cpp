#include "pch.hpp"
#include "EngineCore.hpp"
using namespace Grindstone;

extern "C" {
	ENGINE_CORE_API EngineCore* createEngine(EngineCore::CreateInfo& ci) {
		try {
			EngineCore& core = EngineCore::GetInstance();
			core.Initialize(ci);
			
			return &core;
		}
		catch (std::runtime_error& e) {
			std::cerr << e.what() << std::endl;
			return nullptr;
		}
	}
}
