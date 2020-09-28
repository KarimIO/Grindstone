#include "EditorManager.hpp"
#include <iostream>
using namespace Grindstone;

int main() {
    Grindstone::Editor::Manager editor_manager;
    if (editor_manager.initialize()) {
        editor_manager.run();
    }

    std::cout << "Press any key to quit...";
    std::cin.get();

    return 1;
}