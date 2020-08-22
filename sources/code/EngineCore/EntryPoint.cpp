#include "pch.hpp"
#include "EngineCore.hpp"
using namespace Grindstone;

extern "C" {
    __declspec(dllexport) void runEngine(EngineCore::CreateInfo& ci) {
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