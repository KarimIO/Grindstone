#pragma once

#include "AudioClip.hpp"
#include "AL/al.h"
#include "AL/alc.h"

namespace Grindstone {
	namespace Audio {
		class Source {
		public:
			struct CreateInfo{
				float pitch = 1.f;
				float gain = 1.f;
				float position[3];
				float velocity[3];
				Audio::Clip* audioClip = nullptr;
				bool isLooping = false;
			};
		public:
			Source();
			Source(CreateInfo& createInfo);
			~Source();
			void Play();
			void Pause();
			void SetPitch(float pitch);
			void SetGain(float gain);
			void SetPosition(float x, float y, float z);
			void SetVelocity(float x, float y, float z);
			void SetBuffer(Audio::Clip* audioClip);
			void SetIsLooping(bool isLooping);
			bool IsPlaying();
		private:
			ALuint source;
		};
	}
}
