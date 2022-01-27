#include <iostream>
#include "ModuleLoading.hpp"

namespace Grindstone {
	namespace Utilities {
		namespace Modules {
			Grindstone::Utilities::Modules::Handle Load(std::string name) {
#if defined(_WIN32)
				name = name += ".dll";
				auto library = LoadLibrary(name.c_str());
				return library;
#elif defined(__linux__)
				name = "./lib" + name + ".so";

				void* library = dlopen(name.c_str(), RTLD_NOW);
				if (!library) {
					fprintf(stderr, "dlopen failed: %s\n", dlerror());
					return nullptr;
				};

				return library;
#endif
			}

			void Unload(Grindstone::Utilities::Modules::Handle handle) {
#if defined(_WIN32)
				if (handle)
					FreeLibrary(handle);
#elif defined(__linux__)
				if (handle)
					dlclose(handle);
#endif
			}

			void* GetFunction(Grindstone::Utilities::Modules::Handle module, const char *name) {
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
