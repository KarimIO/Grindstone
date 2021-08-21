#pragma once

#include <string>
#include <vector>
#include "AudioClip.hpp"
#include "AudioSource.hpp"
#include "AL/al.h"
#include "AL/alc.h"

namespace Grindstone {
	namespace Audio {
		class Core {
		public:
			Core();
			~Core();

			virtual bool GetAvailableDevices(std::vector<std::string>& devicesVec, ALCdevice* device);
			virtual Audio::Clip* CreateClip(const char* path);
			virtual Audio::Source* CreateSource(Audio::Source::CreateInfo& createInfo);
		private:
			ALCdevice* device = nullptr;
			ALCcontext* context = nullptr;
		};
	}
}
