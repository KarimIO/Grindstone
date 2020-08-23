#include "ModuleLoading.hpp"

namespace Grindstone {
    namespace Utilities {
        namespace Modules {
            Grindstone::Utilities::Modules::Handle load(std::string name) {
#if defined(_WIN32)
                name = name += ".dll";
                return LoadLibrary(name.c_str());
#elif defined(__linux__)
                name = "./lib" + name + ".so";

                void* dlh = dlopen(name.c_str(), RTLD_NOW);
                if (!dlh) 
                {
                    fprintf(stderr, "dlopen failed: %s\n", dlerror());
                    return nullptr;
                };

                return dlh;
#endif
            }

			void unload(Grindstone::Utilities::Modules::Handle handle) {
#if defined(_WIN32)
                if (handle)
                    FreeLibrary(handle);
#elif defined(__linux__)
                if (handle)
                    dlclose(handle);
#endif
			}

            void* getFunction(Grindstone::Utilities::Modules::Handle module, const char *name) {
#if defined(_WIN32)
                void* fn = GetProcAddress(module, name);
#elif defined(__linux__)
                void* fn = dlsym(module, name);
#endif

                return fn;
            }
        }
    }
}