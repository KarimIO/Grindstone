#include "pch.hpp"

#include <EngineCore/PluginSystem/Interface.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/System.hpp>
#include <EngineCore/BasicComponents.hpp>
#include <Plugins/Renderables3D/StaticMeshComponent.hpp>

using namespace Grindstone;

class TestSys : public ECS::ISystem {
public:
    TestSys(Scene* s);
    virtual void update() override;
private:
    ECS::ComponentArray<TransformComponent>& transform_array_;
};

TestSys::TestSys(Scene* s) : transform_array_(*(ECS::ComponentArray<TransformComponent>*)s->getECS()->getComponentArray("Transform")) {
    scene_ = s;
}

void TestSys::update() {
    return;
    std::cout << "TesSys (" << transform_array_.getCount() << ")\r\n";
    for (size_t i = 0; i < transform_array_.getCount(); ++i) {
        auto& comp = transform_array_[i];
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

        bool use_vulkan = true;
        if (!use_vulkan) {
            plugin_interface->loadPluginCritical("PluginGraphicsOpenGL");
        }
        else {
            plugin_interface->loadPluginCritical("PluginGraphicsVulkan");
        }
        
        plugin_interface->loadPluginCritical("PluginInputSystem");
        /*
        plugin_interface->loadPluginCritical("PluginMaterials");
        plugin_interface->loadPluginCritical("PluginRenderables3D");
        plugin_interface->loadPluginCritical("PluginRenderingDeferred");
        plugin_interface->loadPluginCritical("PluginCamera");
        */

        Window::CreateInfo win_ci;
        win_ci.fullscreen = Window::FullscreenMode::Windowed;
        win_ci.title = "Sandbox";
        win_ci.width = 800;
        win_ci.height = 600;
        win_ci.engine_core = plugin_interface->getEngineCore();
        plugin_interface->enumerateDisplays(&win_ci.display);
        auto win = plugin_interface->createWindow(win_ci);

        GraphicsAPI::Core::CreateInfo gw_create_info;
        gw_create_info.debug = true;
        gw_create_info.window = win;

        auto gw = plugin_interface->getGraphicsCore();
        gw->initialize(gw_create_info);
        std::cout << "Grindstone Graphics: \r\n\t" << gw->getVendorName() << "\r\n\t" << gw->getAdapterName() << "\r\n\t" << gw->getAPIName() << " " << gw->getAPIVersion() << "\r\n===============================\r\n" << std::endl;

        //float clr_color[4] = { 0.f, 0.f, 0.f, 0.f };
        //gw->clear(GraphicsAPI::ClearMode::Color, clr_color);

        win->show();
        plugin_interface->addWindow(win);
        
        // --- For each component

        // Register Components

        // Register Systems
        plugin_interface->registerSystem("TestSys", createTestSys);

        // Load custom game plugins

        auto scene = plugin_interface->getEngineCore()->getSceneManager()->addEmptyScene("My Scene");

        ECS::Entity entity = scene->getECS()->createEntity();
        scene->getECS()->createComponent(entity, "Transform");
        // scene->getECS()->createComponent(entity, "Camera");

        ECS::Entity entity2 = scene->getECS()->createEntity();
        scene->getECS()->createComponent(entity2, "Transform");
        // StaticMesh* mesh = (StaticMesh*)scene->getECS()->createComponent(entity2, "StaticMesh");
    }

    APP_API void releaseModule(Plugins::Interface* plugin_interface) {
    }
    
}