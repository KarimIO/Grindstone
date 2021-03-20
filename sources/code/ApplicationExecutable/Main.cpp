#include <iostream>
#include <Common/Utilities/ModuleLoading.hpp>
#include <EngineCore/EngineCore.hpp>
using namespace Grindstone;

using CreateEngineFunction = EngineCore*(EngineCore::CreateInfo&);

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
	
	Grindstone::Utilities::Modules::unload(handle);
	return 0;
}