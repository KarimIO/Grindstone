#include <iostream>
#include <Common/Utilities/ModuleLoading.hpp>
#include <EngineCore/EngineCore.hpp>
using namespace Grindstone;

using CreateEngineFunction = EngineCore*(EngineCore::CreateInfo&);

#ifdef _WIN32
extern "C" {
	// Request High-Performance GPU for Nvidia and AMD
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main(int argc, char** argv) {
	std::string projectpath = "..";
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-projectpath") == 0 && argc > i + 1) {
			projectpath = argv[i + 1];
		}
	}

	Grindstone::Utilities::Modules::Handle handle;
	handle = Grindstone::Utilities::Modules::Load("EngineCore");

	if (handle == nullptr) {
		std::cerr << "Failed to load EngineCore Module.";
		return 1;
	};

	CreateEngineFunction* createEngineFn =
		(CreateEngineFunction*)Utilities::Modules::GetFunction(handle, "CreateEngine");

	if (createEngineFn == nullptr) {
		std::cerr << "Failed to load CreateEngine in EngineCore Module.";
		return 1;
	}

	EngineCore::CreateInfo createInfo;
	createInfo.isEditor = false;
	createInfo.applicationModuleName = "ApplicationDLL";
	createInfo.applicationTitle = "Grindstone Sandbox";
	createInfo.shouldLoadSceneFromDefaults = true;
	createInfo.scenePath = "";
	createInfo.projectPath = projectpath.c_str();
	EngineCore* engineCore = createEngineFn(createInfo);
	if (engineCore) {
		engineCore->Run();
	}
	
	Grindstone::Utilities::Modules::Unload(handle);
	return 0;
}
