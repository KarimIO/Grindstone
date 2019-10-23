#ifndef _DLL_AUDIO_H
#define _DLL_AUDIO_H

#include "DLLHandler.hpp"

class AudioWrapper;

class DLLAudio : public DLLHandler {
public:
	DLLAudio();
	AudioWrapper *getWrapper();
	void setup();
	void reload();
	~DLLAudio();
private:
	AudioWrapper *wrapper_;
	void(*pfnDeleteAudio)(AudioWrapper*);
};

#endif