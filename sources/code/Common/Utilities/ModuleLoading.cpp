#include <iostream>
#include "ModuleLoading.hpp"

using namespace Grindstone::Utilities;
using namespace Grindstone::Utilities::Modules;

Handle Modules::Load(std::string name) {
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

void Modules::Unload(Handle handle) {
#if defined(_WIN32)
	if (handle)
		FreeLibrary(handle);
#elif defined(__linux__)
	if (handle)
		dlclose(handle);
#endif
}

void* Modules::GetFunction(Handle module, const char *name) {
#if defined(_WIN32)
	void* fn = GetProcAddress(module, name);
#elif defined(__linux__)
	void* fn = dlsym(module, name);
#endif

	return fn;
}
