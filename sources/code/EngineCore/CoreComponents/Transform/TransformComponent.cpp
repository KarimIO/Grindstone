#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "TransformComponent.hpp"
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(TransformComponent)
	REFLECT_STRUCT_MEMBER(position)
	REFLECT_STRUCT_MEMBER(angles)
	REFLECT_STRUCT_MEMBER(scale)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
