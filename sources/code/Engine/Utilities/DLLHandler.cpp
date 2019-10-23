#include "DLLHandler.hpp"
#include <stdexcept>
#include <iostream>
#include "Logger.hpp"

void DLLHandler::initialize(std::string path) {
	#if defined(_WIN32)
		path = path + ".dll";
		handle_ = LoadLibrary(path.c_str());
		if (!handle_) {
			std::string err = "Failed to load " + path + ": " + std::to_string(GetLastError()) + "!";
			throw std::runtime_error(err);
		}
	#elif defined(__linux__)
		path = "./lib"+path+".so";
		handle_ = dlopen(path.c_str(), RTLD_LAZY);
		if (!handle_) {
			std::string err = "Failed to load " + path + ": " + dlerror() + "!";
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

void DLLHandler::close() {
#if defined(_WIN32)
	FreeLibrary(handle_);
#elif defined(__linux__)
	if (handle_)
		dlclose(handle_);
#endif
}

DLLHandler::~DLLHandler() {
	close();
}