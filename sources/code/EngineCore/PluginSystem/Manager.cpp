#include "Manager.hpp"
#include "Interface.hpp"
#include "../Profiling.hpp"

namespace Grindstone {
	namespace Plugins {
		Manager::Manager(EngineCore* engine_core, ECS::Core* core) : interface_(this, core), engine_core_(engine_core) {
		}

		Manager::~Manager() {
			for (auto it = plugins_.rbegin(); it != plugins_.rend(); ++it) {
				auto handle = it->second;
				if (handle) {
					void* f = Grindstone::Utilities::Modules::getFunction(handle, "releaseModule");

					if (f) {
						auto intf = (void (*)(Interface*))f;
						intf(&interface_);
					}
				}
			}
		}

		bool Manager::load(const char *path) {
#ifdef _DEBUG
			std::string profile_str = std::string("Loading module ") + path;
			GRIND_PROFILE_SCOPE(profile_str.c_str());
#endif

			// Return true if plugin already loaded
			auto it = plugins_.find(path);
			if (it != plugins_.end()) {
#ifdef _DEBUG
				throw std::runtime_error("Module already loaded! This error only exists in debug mode to make sure you don't write useless code.");
#endif
				return true;
			}

			auto handle = Grindstone::Utilities::Modules::load(path);

			if (handle) {
				plugins_[path] = handle;

				void* f = Grindstone::Utilities::Modules::getFunction(handle, "initializeModule");

				if (f) {
					auto intf = (void (*)(Interface *))f;
					intf(&interface_);

					return true;
				}
			}

			return false;
		}

		void Manager::loadCritical(const char* path) {
			if (!load(path)) {
				throw std::runtime_error(std::string("Failed to load module: ") + path);
			}
		}

		void Manager::remove(const char* name) {
			auto it = plugins_.find(name);
			if (it != plugins_.end()) {
				auto handle = it->second;
				if (handle) {
					void* f = Grindstone::Utilities::Modules::getFunction(handle, "releaseModule");

					if (f) {
						auto intf = (void (*)(Interface*))f;
						intf(&interface_);
					}
				}

				plugins_.erase(it);
			}
		}
	}
}