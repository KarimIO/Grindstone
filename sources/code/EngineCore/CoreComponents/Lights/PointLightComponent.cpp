#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "PointLightComponent.hpp"
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(PointLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(attenuationRadius)
	REFLECT_STRUCT_MEMBER(intensity)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
