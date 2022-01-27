#pragma once

#include <string>
#include <vector>
#include "Clip.hpp"
#include "Source.hpp"
#include "al.h"
#include "alc.h"

namespace Grindstone {
	namespace Audio {
		class Core {
		public:
			Core();
			~Core();

			static Audio::Core& GetInstance();
			virtual bool GetAvailableDevices(std::vector<std::string>& devicesVec, ALCdevice* device);
			virtual Audio::Clip* CreateClip(const char* path);
			virtual Audio::Source* CreateSource(Audio::Source::CreateInfo& createInfo);
		private:
			ALCdevice* device = nullptr;
			ALCcontext* context = nullptr;
		};
	}
}
