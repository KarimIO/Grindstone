#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "CameraComponent.hpp"
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(CameraComponent)
	REFLECT_STRUCT_MEMBER(isOrthographic)
	REFLECT_STRUCT_MEMBER(near)
	REFLECT_STRUCT_MEMBER(far)
	REFLECT_STRUCT_MEMBER(fov)
	REFLECT_STRUCT_MEMBER(aspectRatio)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
