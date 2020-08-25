#include <Common/Utilities/ModuleLoading.hpp>
#include <EngineCore/EngineCore.hpp>
using namespace Grindstone;

/*
class Res {
public:
    Res(const char* name);
    void addDependency(Res* res);
    const char* getName();
    std::vector<Res*> dependencies_;
private:
    std::string name_;
};

const char* Res::getName() {
    return name_.c_str();
}

Res::Res(const char* name) : name_(name) {}

void Res::addDependency(Res* res) {
    dependencies_.push_back(res);
}

void traverseDependencies(Res* res, std::vector<Res*> &accessed) {
    std::cout << res->getName() << std::endl;

    for (Res* r : res->dependencies_) {
        bool found = false;
        for (Res* e : accessed) {
            if (r == e) {
                found = true;
                break;
            }
        }

        if (!found) {
            traverseDependencies(r, accessed);
        }
    }

    accessed.push_back(res);
}
*/

int main() {
    /*std::vector<Res*> accessed;

    Res a("a");
    Res b("b");
    Res c("c");
    Res d("d");
    Res e("e");

    e.addDependency(&d);
    d.addDependency(&c);
    d.addDependency(&b);
    c.addDependency(&b);
    b.addDependency(&a);
    
    traverseDependencies(&e, accessed);

    return 0;*/

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