#include <iostream>
#include <Common/Utilities/ModuleLoading.hpp>
#include <EngineCore/EngineCore.hpp>
using namespace Grindstone;

#ifdef _WIN32
extern "C" {
	// Request High-Performance GPU for Nvidia and AMD
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main(int argc, char** argv) {
	try {
		std::string projectPath = std::filesystem::current_path().parent_path().string();
		for (int i = 1; i < argc; ++i) {
			if (strcmp(argv[i], "-projectpath") == 0 && argc > i + 1) {
				projectPath = argv[i + 1];
			}
		}

		Grindstone::Utilities::Modules::Handle handle;
		handle = Grindstone::Utilities::Modules::Load("EngineCore");

		if (handle == nullptr) {
			std::cerr << "Failed to load EngineCore Module.";
			return 1;
		};

		using CreateEngineFunction = EngineCore*();
		CreateEngineFunction* createEngineFn =
			(CreateEngineFunction*)Utilities::Modules::GetFunction(handle, "CreateEngine");

		if (createEngineFn == nullptr) {
			std::cerr << "Failed to load CreateEngine in EngineCore Module.";
			return 1;
		}

		EngineCore::EarlyCreateInfo earlyCreateInfo;
		earlyCreateInfo.isEditor = false;
		earlyCreateInfo.assetLoader = nullptr;
		earlyCreateInfo.applicationModuleName = "ApplicationDLL";
		earlyCreateInfo.applicationTitle = "Grindstone Sandbox";
		earlyCreateInfo.projectPath = projectPath.c_str();
		std::string currentPath = (std::filesystem::path(projectPath) / "bin").string();
		earlyCreateInfo.engineBinaryPath = currentPath.c_str();
		EngineCore* engineCore = createEngineFn();

		if (engineCore->EarlyInitialize(earlyCreateInfo)) {
			std::cerr << "Failed to initialize EngineCore Module.";
			return 1;
		}

		EngineCore::LateCreateInfo lateCreateInfo{};

		if (engineCore->Initialize(lateCreateInfo)) {
			std::cerr << "Failed to initialize EngineCore Module.";
			return 1;
		}

		engineCore->InitializeScene(true);
		engineCore->ShowMainWindow();

		if (engineCore) {
			engineCore->Run();
		}

		using DestroyEngineFunction = void * ();
		DestroyEngineFunction* destroyEngineFn =
			(DestroyEngineFunction*)Utilities::Modules::GetFunction(handle, "DestroyEngine");

		if (destroyEngineFn != nullptr) {
			destroyEngineFn();
		}
		else {
			std::cerr << "Failed to load DestroyEngine in EngineCore Module.";
		}

		Grindstone::Utilities::Modules::Unload(handle);
	}
	catch (std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		OutputDebugString(e.what());
	}

	return 0;
}
