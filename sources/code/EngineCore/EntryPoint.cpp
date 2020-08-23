#include "pch.hpp"
#include "EngineCore.hpp"
using namespace Grindstone;

extern "C" {
    ENGINE_CORE_API void runEngine(EngineCore::CreateInfo& ci) {
        try {
            EngineCore core;
            core.initialize(ci);
            core.run();
        }
        catch (std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}