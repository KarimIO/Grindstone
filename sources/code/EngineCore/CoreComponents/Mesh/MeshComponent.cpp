#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "MeshComponent.hpp"
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(MeshComponent)
	REFLECT_STRUCT_MEMBER(meshPath)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
