#ifndef _SOUNDFILE_COMMON_H
#define _SOUNDFILE_COMMON_H

class SoundFile {
public:
	virtual void Play() = 0;
	virtual void PlayLoop(int id) = 0;
	virtual void Stop() = 0;
};

#endif