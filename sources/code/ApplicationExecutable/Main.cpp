#include <iostream>
#include <Common/Utilities/ModuleLoading.hpp>
#include <EngineCore/EngineCore.hpp>
using namespace Grindstone;

using CreateEngineFunction = EngineCore*(EngineCore::CreateInfo&);

#ifdef _WIN32
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main() {
	Grindstone::Utilities::Modules::Handle handle;
	handle = Grindstone::Utilities::Modules::load("EngineCore");

	if (handle == nullptr) {
		std::cerr << "Failed to load EngineCore Module.";
		return 1;
	};

	CreateEngineFunction* createEngineFn =
		(CreateEngineFunction*)Utilities::Modules::getFunction(handle, "createEngine");

	if (createEngineFn == nullptr) {
		std::cerr << "Failed to load runEngine in EngineCore Module.";
		return 1;
	}

	EngineCore::CreateInfo create_info;
	create_info.isEditor = false;
	create_info.applicationModuleName = "ApplicationDLL";
	create_info.applicationTitle = "Grindstone Sandbox";
	EngineCore* engineCore = createEngineFn(create_info);
	engineCore->Run();
	
	Grindstone::Utilities::Modules::unload(handle);
	return 0;
}
