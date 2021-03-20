#include "EditorManager.hpp"
#include <iostream>
using namespace Grindstone;

int main() {
    Grindstone::Editor::Manager editorManager;
    editorManager.initialize();
    editorManager.run();

    return 1;
}