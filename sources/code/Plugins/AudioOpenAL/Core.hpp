#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "Source.hpp"
#include "al.h"
#include "alc.h"

namespace Grindstone::Audio {
	class Core {
	public:
		Core();
		~Core();

		static Audio::Core& GetInstance();
		virtual bool GetAvailableDevices(std::vector<std::string>& devicesVec, ALCdevice* device);
	private:
		ALCdevice* device = nullptr;
		ALCcontext* context = nullptr;
		static Core* instance;
	};
}
