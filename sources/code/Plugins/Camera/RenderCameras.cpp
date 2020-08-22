#include "Camera.hpp"
#include <EngineCore/BasicComponents.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace Grindstone;

void render() {
    // For each camera
    {
        CameraComponent camera;
        TransformComponent transform;

        glm::mat4 perspective, view;
        perspective = glm::perspective(
            camera.fov_, // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
            4.0f / 3.0f,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
            camera.near_,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
            camera.far_             // Far clipping plane. Keep as little as possible.
        );

        view = glm::lookAt(
            transform.position_, // the position of your camera, in world space
            transform.getForward(),   // where you want to look at, in world space
            transform.getUp()       // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
        );

        // Update uniform buffer

        // Cull geometry

        // Render all renderpasses
        
    }
}