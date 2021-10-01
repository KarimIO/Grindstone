#include "pch.hpp"
#include "EngineCore.hpp"

#ifdef _MSC_VER
#include <debugapi.h>
#endif

using namespace Grindstone;

extern "C" {
	ENGINE_CORE_API EngineCore* createEngine(EngineCore::CreateInfo& ci) {
		try {
			EngineCore& core = EngineCore::GetInstance();
			core.Initialize(ci);
			
			return &core;
		}
		catch (std::runtime_error& e) {

#ifdef _MSC_VER
			OutputDebugString(e.what());
#endif

			std::cerr << e.what() << std::endl;
			return nullptr;
		}
	}
}
