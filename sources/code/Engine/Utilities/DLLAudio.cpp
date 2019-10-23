#include "DLLAudio.hpp"

DLLAudio::DLLAudio() {
	setup();
}

AudioWrapper *DLLAudio::getWrapper() {
	return wrapper_;
}

void DLLAudio::setup() {
	std::string library = "audioopenal";

	initialize(library);
}

void DLLAudio::reload() {
	/*if (wrapper_) {
		pfnDeleteAudio(wrapper_);
		wrapper_ = nullptr;
	}*/

	close();

	setup();
}

DLLAudio::~DLLAudio() {
	/*if (wrapper_) {
		pfnDeleteAudio(wrapper_);
		wrapper_ = nullptr;
	}*/
}