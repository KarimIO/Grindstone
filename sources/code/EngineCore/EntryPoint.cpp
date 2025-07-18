#include "pch.hpp"
#include "EngineCore.hpp"

#ifdef _MSC_VER
#include <Windows.h>
#endif

using namespace Grindstone;

extern "C" {
	ENGINE_CORE_API EngineCore* CreateEngine(EngineCore::CreateInfo& ci) {
		try {
			EngineCore* core = new EngineCore();
			EngineCore::SetInstance(*core);
			core->Initialize(ci);

			return core;
		}
		catch (std::runtime_error& e) {

#ifdef _MSC_VER
			OutputDebugString(e.what());
#endif

			std::cerr << e.what() << '\n';
			return nullptr;
		}
	}

	ENGINE_CORE_API void DestroyEngine(void* engineCore) {
		delete static_cast<EngineCore*>(&EngineCore::GetInstance());
	}
}
