#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "Source.hpp"
#include "al.h"
#include "alc.h"

namespace Grindstone {
	class EngineCore;

	namespace Audio {
		class Core {
		public:
			Core();
			~Core();

			void SetEngineCorePtr(EngineCore* engineCore);
			static Audio::Core& GetInstance();
			virtual bool GetAvailableDevices(std::vector<std::string>& devicesVec, ALCdevice* device);
			EngineCore* engineCore = nullptr;
		private:
			ALCdevice* device = nullptr;
			ALCcontext* context = nullptr;
			static Core* instance;
		};
	}
}
