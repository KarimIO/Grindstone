#include <Common/Utilities/ModuleLoading.hpp>
#include <EngineCore/EngineCore.hpp>
using namespace Grindstone;

int main() {
    // Load Engine DLL
    Grindstone::Utilities::Modules::Handle handle;
    handle = Grindstone::Utilities::Modules::load("EngineCore");

    if (handle) {
        void* f = Grindstone::Utilities::Modules::getFunction(handle, "runEngine");
        if (f) {
            auto fptr = (void (*)(EngineCore::CreateInfo&))f;

            EngineCore::CreateInfo create_info;
            create_info.is_editor_ = false;
            create_info.application_module_name_ = "ApplicationDLL";
            create_info.application_title_ = "Grindstone Sandbox";
            fptr(create_info);

            Grindstone::Utilities::Modules::unload(handle);
            return 0;
        }
        else {
            std::cerr << "Failed to load runEngine in EngineCore Module.";
        }
    }
    else {
        std::cerr << "Failed to load EngineCore Module.";
    }

    std::cout << "Press any key to quit...";
    std::cin.get();

    return 1;
}