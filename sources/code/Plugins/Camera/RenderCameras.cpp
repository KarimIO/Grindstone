#include "RenderCameras.hpp"
#include <EngineCore/Scenes/Scene.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
using namespace Grindstone;

CameraRenderingSystem::CameraRenderingSystem(Scene* s) : transform_array_(*(ECS::ComponentArray<TransformComponent>*)s->getECS()->getComponentArray("Transform")), camera_array_(*(ECS::ComponentArray<CameraComponent>*)s->getECS()->getComponentArray("Camera")) {
    scene_ = s;
}

void CameraRenderingSystem::setGraphicsCore(GraphicsAPI::Core* core) {
    graphics_core_ = core;
}

void CameraRenderingSystem::update() {
    // For each camera
    {
        CameraComponent& camera = camera_array_.getComponent(1);
        TransformComponent& transform = transform_array_.getComponent(1);

        glm::mat4 perspective, view;
        perspective = glm::perspective(
            camera.fov_, // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
            4.0f / 3.0f,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
            camera.near_,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
            camera.far_             // Far clipping plane. Keep as little as possible.
        );

        view = glm::lookAt(
            glm::vec3(1,1,1), // the position of your camera, in world space
            glm::vec3(0,0,0),   // where you want to look at, in world space
            glm::vec3(0,1,0)       // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
        );

        // Update uniform buffer

        // Cull geometry

        // Render all renderpasses

    }
}