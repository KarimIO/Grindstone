#include "Common/Math.hpp"
#include "ScriptComponent.hpp"
using namespace Grindstone::Scripting::CSharp;

REFLECT_STRUCT_BEGIN(ScriptComponent)
	REFLECT_STRUCT_MEMBER(assembly)
	REFLECT_STRUCT_MEMBER(scriptNamespace)
	REFLECT_STRUCT_MEMBER(scriptClass)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
