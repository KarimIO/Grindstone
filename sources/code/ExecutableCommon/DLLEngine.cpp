#include "DLLEngine.hpp"
#include <stdexcept>

void DLLEngine::initializeDLL() {
    initialize("Engine");

    launchEngine = (void *(*)())getFunction("launchEngine");
    if (!launchEngine) {
        throw std::runtime_error("Cannot find launch engine function!\n");
    }

    deleteEngine = (void (*)(void *))getFunction("deleteEngine");
    if (!deleteEngine) {
        throw std::runtime_error("Cannot find delete engine function!\n");
    }
}
