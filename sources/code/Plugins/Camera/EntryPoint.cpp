#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/ECS/ComponentArray.hpp>
#include "Camera.hpp"
#include "RenderCameras.hpp"
using namespace Grindstone;

ECS::IComponentArray* cameraFactory() {
    return new ECS::ComponentArray<CameraComponent>();
}

ECS::ISystem* renderCameraSystemFactory(Scene* s) {
    return new CameraRenderingSystem(s);
}

extern "C" {
    GRAPHICS_OPENGL_API void initializeModule(Plugins::Interface* plugin_interface) {
        plugin_interface->registerComponentType("Camera", cameraFactory);
        plugin_interface->registerSystem("RenderCameras", renderCameraSystemFactory);
    }

    GRAPHICS_OPENGL_API void releaseModule(Plugins::Interface* plugin_interface) {
    }
}