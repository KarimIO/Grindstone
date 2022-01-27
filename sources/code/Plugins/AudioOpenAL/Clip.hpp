#pragma once

#include <vector>
#include "al.h"

namespace Grindstone {
	namespace Audio {
		class Clip {
		public:
			Clip() = default;
			Clip(const char* path);
			~Clip();

			ALuint GetOpenALBuffer();
			void LoadFromPath(const char* path);
		private:
			void LoadWavFromPath(const char* path);
			void CreateOpenALBuffer(const char* bufferPtr, size_t bufferSize);
			ALuint buffer = -1;
			std::uint32_t channelCount = 0;
			std::uint32_t sampleRate = 0;
			std::uint8_t bitsPerSample = 0;
		};
	}
}