#include "Camera.hpp"
#include <EngineCore/BasicComponents.hpp>
#include <EngineCore/ECS/System.hpp>
#include <EngineCore/ECS/ComponentArray.hpp>
#include <Common/Graphics/Core.hpp>

using namespace Grindstone;

class CameraRenderingSystem : public ECS::ISystem {
public:
    CameraRenderingSystem(Scene* s);
    void setGraphicsCore(GraphicsAPI::Core* core, Window* win);
    virtual void update() override;
private:
    ECS::ComponentArray<CameraComponent>& camera_array_;
    ECS::ComponentArray<TransformComponent>& transform_array_;
    GraphicsAPI::Core* graphics_core_;
    Window* window_;
};
