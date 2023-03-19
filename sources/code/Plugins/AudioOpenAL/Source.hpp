#pragma once

#include "AudioClip.hpp"
#include "al.h"
#include "alc.h"

namespace Grindstone {
	namespace Audio {
		class Source {
		public:
			struct CreateInfo{
				float volume = 1.f;
				float pitch = 1.f;
				float position[3];
				float velocity[3];
				Audio::AudioClipAsset* audioClip = nullptr;
				bool isLooping = false;
			};
		public:
			Source();
			Source(CreateInfo& createInfo);
			~Source();
			void Play();
			void Pause();
			void SetVolume(float volume);
			void SetPitch(float pitch);
			void SetPosition(float x, float y, float z);
			void SetVelocity(float x, float y, float z);
			void SetBuffer(Audio::AudioClipAsset* audioClip);
			void SetIsLooping(bool isLooping);
			bool IsPlaying();
		private:
			ALuint source;
		};
	}
}
