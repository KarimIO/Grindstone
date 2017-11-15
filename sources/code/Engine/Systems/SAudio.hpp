#ifndef _S_AUDIO_H
#define _S_AUDIO_H

#include "SoundFile.hpp"

class CAudio {
private:
	SoundFile *sound;
public:
	void SetSound(SoundFile *sound);
	void Play();
	void Stop();
};

class SAudio {
public:

};

#endif