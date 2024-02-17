#include "pch.hpp"
#include "EngineCore.hpp"

#ifdef _MSC_VER
#include <Windows.h>
#endif

using namespace Grindstone;

extern "C" {
	ENGINE_CORE_API EngineCore* CreateEngine(EngineCore::CreateInfo& ci) {
		try {
			EngineCore& core = EngineCore::GetInstance();
			core.Initialize(ci);

			return &core;
		}
		catch (std::runtime_error& e) {

#ifdef _MSC_VER
			OutputDebugString(e.what());
#endif

			std::cerr << e.what() << '\n';
			return nullptr;
		}
	}
}
