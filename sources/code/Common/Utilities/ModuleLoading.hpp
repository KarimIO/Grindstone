#pragma once

#include <string>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

namespace Grindstone {
    namespace Utilities {
        namespace Modules {
#if defined(_WIN32)
            typedef HMODULE Handle;
#elif defined(__linux__)
            typedef void* Handle;
#endif

            Grindstone::Utilities::Modules::Handle load(std::string name);
            void unload(Grindstone::Utilities::Modules::Handle handle);

            void* getFunction(Grindstone::Utilities::Modules::Handle handle, const char *name);
        }
    }
}