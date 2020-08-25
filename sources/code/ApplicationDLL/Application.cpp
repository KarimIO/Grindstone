#include "pch.hpp"

#include <EngineCore/PluginSystem/Interface.hpp>
#include <Common/Graphics/GraphicsWrapper.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/System.hpp>
#include <EngineCore/BasicComponents.hpp>

using namespace Grindstone;

class TestSys : public ECS::ISystem {
public:
    TestSys(Scene* s);
    virtual void update() override;
};

TestSys::TestSys(Scene* s) {
    scene_ = s;
}

void TestSys::update() {
    std::cout << "TesSys\r\n";
    ECS::IComponentArray* comp_arr_generic = scene_->getECS()->getComponentArray("Transform");
    ECS::ComponentArray<TransformComponent>& comp_arr = *(ECS::ComponentArray<TransformComponent>*)comp_arr_generic;
    std::cout << "\t" << comp_arr.getCount() << "\r\n";
    for (size_t i = 0; i < comp_arr.getCount(); ++i) {
        auto& comp = comp_arr[i];
        std::cout << "\t" << comp.position_[0] << " " << comp.position_[1] << " " << comp.position_[2] << "\r\n";
    }
}

ECS::ISystem* createTestSys(Scene* s) {
    return new TestSys(s);
}

extern "C" {
    APP_API void initializeModule(Plugins::Interface* plugin_interface) {
        // Load engine plugins
        // plugin_interface->loadPluginCritical("ScriptCSharp");
        plugin_interface->loadPluginCritical("PluginGraphicsOpenGL");

        /*Window::CreateInfo win_ci;
        win_ci.fullscreen = Window::FullscreenMode::Borderless;
        win_ci.title = "Sandbox";
        win_ci.width = 1000;
        win_ci.height = 1000;
        plugin_interface->enumerateDisplays(&win_ci.display);
        auto win = plugin_interface->createWindow(win_ci);

        GraphicsAPI::Core::CreateInfo gw_create_info;
        gw_create_info.debug = true;
        gw_create_info.window = win;

        auto gw = plugin_interface->getGraphicsCore();
        gw->initialize(gw_create_info);

        float clr_color[4] = { 0.f, 0.f, 0.f, 0.f };
        gw->clear(GraphicsAPI::ClearMode::Color, clr_color);

        win->show();
        plugin_interface->addWindow(win);*/
        
        // --- For each component

        // Register Components

        // Register Systems
        plugin_interface->registerSystem("TestSys", createTestSys);

        // Load custom game plugins

        auto scene = plugin_interface->getEngineCore()->getSceneManager()->addEmptyScene("My Scene");
        ECS::Entity entity = scene->getECS()->createEntity();
        scene->getECS()->createComponent(entity, "Transform");
    }

    APP_API void releaseModule(Plugins::Interface* plugin_interface) {
    }
    
}