#include "SAudio.hpp"

void CAudio::SetSound(SoundFile * sound) {
	this->sound = sound;
}

void CAudio::Play() {
	sound->PlayLoop(-1);
}

void CAudio::Stop() {
	sound->Stop();
}
