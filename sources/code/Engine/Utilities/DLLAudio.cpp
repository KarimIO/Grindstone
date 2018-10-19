#include "DLLAudio.hpp"

DLLAudio::DLLAudio() {
	std::string library = "audioopenal";
	
	initialize(library);
}

AudioWrapper *DLLAudio::getWrapper() {
	return wrapper_;
}

DLLAudio::~DLLAudio() {
	if (wrapper_) {
		pfnDeleteAudio(wrapper_);
		wrapper_ = nullptr;
	}
}