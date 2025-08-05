#pragma once

#include <string>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

namespace Grindstone::Utilities::Modules {
#if defined(_WIN32)
	using Handle = HMODULE;
	using BinarySearchDirectoryHandle = DLL_DIRECTORY_COOKIE;
#elif defined(__linux__)
	using Handle = void*;
	using BinarySearchDirectoryHandle = void*;
#endif

	Grindstone::Utilities::Modules::Handle Load(std::string name);
	void Unload(Grindstone::Utilities::Modules::Handle handle);

	void* GetFunction(Grindstone::Utilities::Modules::Handle handle, const char *name);
}
