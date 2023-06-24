#include <string>
#include <mono/metadata/class.h>
#include <mono/jit/jit.h>

#include "ScriptClass.hpp"
using namespace Grindstone::Scripting::CSharp;

ScriptField::ScriptField(std::string name, MonoClassField* classFieldPtr) : name(name), classFieldPtr(classFieldPtr) {}

void ScriptField::Get(MonoObject* monoObject, void* outValue) {
	mono_field_get_value(monoObject, classFieldPtr, outValue);
}

void ScriptField::Set(MonoObject* monoObject, void* outValue) {
	mono_field_set_value(monoObject, classFieldPtr, outValue);
}

Grindstone::Scripting::CSharp::ScriptClass::ScriptClass(
	std::string scriptNamespace,
	std::string scriptClassname,
	MonoClass* monoClass
) :
	scriptNamespace(scriptNamespace),
	scriptClassname(scriptClassname),
	monoClass(monoClass)
{
	methods.constructor = mono_class_get_method_from_name(monoClass, ".ctor", 0);
	methods.onAttachComponent = mono_class_get_method_from_name(monoClass, "OnAttachComponent", 0);
	methods.onStart = mono_class_get_method_from_name(monoClass, "OnStart", 0);
	methods.onUpdate = mono_class_get_method_from_name(monoClass, "OnUpdate", 0);
	methods.onEditorUpdate = mono_class_get_method_from_name(monoClass, "OnEditorUpdate", 0);
	methods.onDelete = mono_class_get_method_from_name(monoClass, "OnDelete", 0);
}
