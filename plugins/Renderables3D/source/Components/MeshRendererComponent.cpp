#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Renderables3D/include/Components/MeshRendererComponent.hpp>
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(MeshRendererComponent)
	REFLECT_STRUCT_MEMBER(materials)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
