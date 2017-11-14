#include "Entity.h"
#include "../Core/Engine.h"

Entity::Entity() : id((uint32_t)engine.entities.size()) {
}