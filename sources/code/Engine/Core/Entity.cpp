#include "Entity.hpp"
#include "Engine.hpp"

Entity::Entity() : id_((uint32_t)engine.entities.size()), components_{UINT32_MAX} {
}

Entity::~Entity() {
    /*if (id_[COMPONENT_TRANSFORM] != UINT32_MAX)
        engine.transformSystem.RemoveComponent(id_[COMPONENT_TRANSFORM]);
    
    if (id_[COMPONENT_CONTROLLER] != UINT32_MAX)
        engine.controllerSystem.RemoveComponent(id_[COMPONENT_CONTROLLER]);
    
    if (id_[COMPONENT_CAMERA] != UINT32_MAX)
        engine.cameraSystem.RemoveComponent(id_[COMPONENT_CAMERA]);
    
    if (id_[COMPONENT_GEOMETRY] != UINT32_MAX)
        engine.geometry_system.RemoveComponent(id_[COMPONENT_GEOMETRY]);
    
    if (id_[COMPONENT_LIGHT_POINT] != UINT32_MAX)
        engine.lightSystem.RemoveComponent(COMPONENT_LIGHT_POINT, id_[COMPONENT_TRANSFORM]);
    
    if (id_[COMPONENT_LIGHT_SPOT] != UINT32_MAX)
        engine.lightSystem.RemoveComponent(COMPONENT_LIGHT_SPOT, id_[COMPONENT_TRANSFORM]);
    
    if (id_[COMPONENT_LIGHT_DIRECTIONAL] != UINT32_MAX)
        engine.lightSystem.RemoveComponent(COMPONENT_LIGHT_DIRECTIONAL, id_[COMPONENT_TRANSFORM]);
    
    if (id_[COMPONENT_PHYSICS] != UINT32_MAX)
        engine.transformSystem.RemoveComponent(id_[COMPONENT_PHYSICS]);
    
    if (id_[COMPONENT_INPUT] != UINT32_MAX)
        engine.inputSystem.RemoveComponent(id_[COMPONENT_INPUT]);
    
    if (id_[COMPONENT_AUDIO] != UINT32_MAX)
        engine.audio_system_.RemoveComponent(id_[COMPONENT_AUDIO]);
    
    if (id_[COMPONENT_SCRIPT] != UINT32_MAX)
        engine.script_system_.RemoveComponent(id_[COMPONENT_SCRIPT]);
    
    if (id_[COMPONENT_GAME_LOGIC] != UINT32_MAX)
        engine.gameplay_system.RemoveComponent(id_[COMPONENT_GAME_LOGIC]);*/
}