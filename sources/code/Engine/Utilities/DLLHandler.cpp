#include "DLLHandler.hpp"
#include <stdexcept>
#include <iostream>

void DLLHandler::initialize(std::string path) {
	#if defined(_WIN32)
		handle_ = LoadLibrary((path+".dll").c_str());
		if (!handle_) {
			std::string err = "Failed to load " + path + "!";
			throw std::runtime_error(err);
		}
	#elif defined(__linux__)
		path = "./lib"+path+".so";
		handle_ = dlopen(path.c_str(), RTLD_LAZY);
		if (!handle_) {
			std::string err = "Failed to load " + path + ": " + dlerror();
			throw std::runtime_error(err);
		}
	#endif
}

void *DLLHandler::getFunction(std::string name) {
	#if defined(_WIN32)
		void *fn = GetProcAddress(handle_, name.c_str());
	#elif defined(__linux__)
		void *fn = dlsym(handle_, name.c_str());
	#endif
	
	if (!fn) {
		throw std::runtime_error("Error loading function.\n");
		return nullptr;
	}

	return fn;
}

DLLHandler::~DLLHandler() {
	#if defined(_WIN32)
		FreeLibrary(handle_);
	#elif defined(__linux__)
		if (handle_)
			dlclose(handle_);
	#endif
}