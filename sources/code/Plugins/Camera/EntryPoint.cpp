#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/ECS/ComponentArray.hpp>
#include <EngineCore/EngineCore.hpp>
#include "Camera.hpp"
#include "RenderCameras.hpp"
using namespace Grindstone;

Plugins::Interface* g_plugin_interface;

ECS::IComponentArray* cameraFactory() {
    return new ECS::ComponentArray<CameraComponent>();
}

ECS::ISystem* renderCameraSystemFactory(Scene* s) {
    auto c = new CameraRenderingSystem(s);
    auto w = g_plugin_interface->getEngineCore()->windows_[0];
    c->setGraphicsCore(g_plugin_interface->getGraphicsCore(), w);
    return c;
}

extern "C" {
    CAMERA_API void initializeModule(Plugins::Interface* plugin_interface) {
        g_plugin_interface = plugin_interface;

        plugin_interface->registerComponentType("Camera", cameraFactory);
        plugin_interface->registerSystem("RenderCameras", renderCameraSystemFactory);

    }

    CAMERA_API void releaseModule(Plugins::Interface* plugin_interface) {
    }
}