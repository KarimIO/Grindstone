#include "EBase.h"
#include "../Core/Engine.h"

EBase::EBase() {
	id = (uint32_t)engine.entities.size();
}