#include "pch.hpp"
#include "EngineCore.hpp"

int main() {
    Grindstone::EngineCore core;
    core.initialize();
    core.run();
    return 0;
}