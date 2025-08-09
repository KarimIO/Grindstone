#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <Grindstone.Renderables.3D//include/Components/MeshComponent.hpp>
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(MeshComponent)
	REFLECT_STRUCT_MEMBER(mesh)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
