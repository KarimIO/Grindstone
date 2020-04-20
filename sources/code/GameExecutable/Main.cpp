#include <ExecutableCommon/DLLEngine.hpp>
#include <Engine/Core/Engine.hpp>
#include <Engine/Core/Space.hpp>
#include <WindowModule/Win32Window.hpp>

int main() {
    try {
        DLLEngine dll_engine;
        dll_engine.initializeDLL();
        Engine *app = (Engine *)dll_engine.launchEngine();
        WindowCreateInfo windowCreateInfo;
        windowCreateInfo.fullscreen = WindowFullscreenMode::Windowed;
        windowCreateInfo.width = 1366;
        windowCreateInfo.height = 768;
        windowCreateInfo.title = "Grindstone";
        windowCreateInfo.input_interface = nullptr;
        auto window = new Win32Window();
        window->initialize(windowCreateInfo);

        app->initialize(window);

        std::string level_name = "../assets/scenes/sponza.json";
        Space *space = app->addSpace("Main Game");
        space->addScene(level_name.c_str());

        window->show();
        window->setWindowFocus();

        while (!app->shouldQuit()) {
            app->run();
        }
        //dll_engine.deleteEngine(app);

    }
    catch (std::runtime_error & e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}