#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "TagComponent.hpp"
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(TagComponent)
	REFLECT_STRUCT_MEMBER(tag)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
