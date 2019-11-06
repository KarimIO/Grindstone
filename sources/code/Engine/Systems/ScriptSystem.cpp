#include "ScriptSystem.hpp"

#include "Core/Engine.hpp"
#include "Core/Scene.hpp"
#include "Core/Space.hpp"

#include <iostream>

#include <mono/jit/jit.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/assembly.h>


ScriptInstance::ScriptInstance(ScriptClass* c, MonoObject *o) : script_class_(c), script_object_(o) {}

AssemblyPackage::AssemblyPackage(MonoAssembly *a, MonoImage *i, std::string p) : assembly_(a), mono_image_(i), path_(p), classes_{} {}

ScriptFields::ScriptFields(ScriptFields::ScriptFieldType t, std::string n, MonoClassField *f) : type_(t), name_(n), field_(f) {}

ScriptComponent::ScriptComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_SCRIPT, object_handle, id) {}

ScriptSystem::ScriptSystem() : System(COMPONENT_SCRIPT) {
	mono_set_dirs("C:\\Program Files\\Mono\\lib", "C:\\Program Files\\Mono\\etc");

	domain_ = mono_jit_init_version("grindstone_mono_domain",
		"v4.0.30319");

	auto i = loadAssembly("scriptbin/Main.dll");
	loadClass(0, "Example");
}

unsigned int ScriptSystem::loadAssembly(std::string path) {
	MonoAssembly *assembly = mono_domain_assembly_open(domain_, path.c_str());
	MonoImage *image = mono_assembly_get_image(assembly);

	assemblies_.emplace_back(new AssemblyPackage(assembly, image, path));

	return (unsigned int)assemblies_.size() - 1;
}

MonoDomain *ScriptSystem::getDomain() {
	return domain_;
}

ScriptSystem::~ScriptSystem() {
	mono_jit_cleanup(domain_);
}

ScriptFields::ScriptFieldType getTypeToken(std::string t) {
	if (t == "System.Boolean") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_BOOL;
	else if (t == "System.String") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_STRING;
	else if (t == "System.UInt64") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_UINT64;
	else if (t == "System.Int64") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_INT64;
	else if (t == "System.UInt32") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_UINT32;
	else if (t == "System.Int32") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_INT32;
	else if (t == "System.UInt16") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_UINT16;
	else if (t == "System.Int16") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_INT16;
	else if (t == "System.Byte") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_UINT8;
	else if (t == "System.SByte") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_INT8;
	else if (t == "System.Double") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_DOUBLE;
	else if (t == "System.Single") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_FLOAT;
	else if (t == "System.Char") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_CHAR;
	else if (t == "System.Object") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_OBJECT;
	else if (t == "System.Decimal") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_DECIMAL;
	else if (t == "System.DateTime") return ScriptFields::ScriptFieldType::SCRIPT_FIELD_DATETIME;
	else return ScriptFields::ScriptFieldType::SCRIPT_FIELD_UNKNOWN;
}

void ScriptSystem::loadClass(unsigned int assembly_id, std::string classname) {
	auto assembly = assemblies_[assembly_id];
	auto script_class = new ScriptClass();
	script_class->classname_ = classname;
	assembly->classes_.push_back(script_class);
	script_classes_[classname] = (script_class);
	
	// find the Entity class in the image
	auto mono_class = script_class->class_ = mono_class_from_name(assembly->mono_image_, "", classname.c_str());

	// allocate memory for one Entity instance
	//script_object_ = mono_object_new(domain, script_class_);

	// find the constructor method that takes one parameter

	//MonoObject* exception = nullptr;
	//mono_runtime_invoke(script_method_constructor_, script_object_, nullptr, &exception);

	MonoClassField *field = nullptr;
	void *iter = nullptr;
	while (field = mono_class_get_fields(mono_class, &iter)) {
		// if (mono_access
		script_class->fields_.emplace_back(getTypeToken(mono_type_get_name(mono_field_get_type(field))), mono_field_get_name(field), field);
		std::cout << mono_type_get_name(mono_field_get_type(field)) << " " << script_class->classname_ << "::" << mono_field_get_name(field) << "(" << mono_field_get_flags(field) << ")" << std::endl;
	}

	script_class->script_method_ctor_	= mono_class_get_method_from_name(mono_class, ".ctor", 0);
	script_class->script_method_init_	= mono_class_get_method_from_name(mono_class, "initialize", 0);
	script_class->script_method_start_	= mono_class_get_method_from_name(mono_class, "start", 0);
	script_class->script_method_update_ = mono_class_get_method_from_name(mono_class, "update", 0);
	script_class->script_method_cleanup_= mono_class_get_method_from_name(mono_class, "cleanup", 0);
	// check for any thrown exception
	/*if (exception) {
		std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr))
			<< std::endl;
	}*/

	// create a MonoString that will be passed to the constructor as an argument
	/*MonoString* name = mono_string_new(mono_domain_get(), "Giorgos");
	void* args[1];
	args[0] = name;

	// finally, invoke the constructor
	
	// find the Process method that takes zero parameters
	MonoMethod* processMethod = mono_class_get_method_from_name(entityClass,
		"Process",
		0);
	exception = nullptr;

	// invoke the method
	// if invoking static methods, then the second argument must be null
	mono_runtime_invoke(processMethod, entityObject, nullptr, &exception);

	// check for any thrown exception
	if (exception)
	{
		std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr))
			<< std::endl;
	}

	// find the GetName method
	MonoMethod* getNameMethod = mono_class_get_method_from_name(entityClass,
		"GetName",
		0);
	exception = nullptr;
	MonoString* ret = (MonoString*)mono_runtime_invoke(getNameMethod, entityObject, nullptr, &exception);
	char* c = mono_string_to_utf8(ret);
	std::cout << "Value of 'Name' is " << c << std::endl;
	// free the memory allocated from mono_string_to_utf8 ()
	mono_free(c);

	// find the Id field in the Entity class
	MonoClassField* idField = mono_class_get_field_from_name(entityClass, "Id");
	int value = 42;

	// set the field's value
	mono_field_set_value(entityObject, idField, &value);

	int result;
	mono_field_get_value(entityObject, idField, &result);
	std::cout << "Value of 'Id' is " << result << std::endl;*/
}

inline ScriptClass *ScriptSystem::getClass(std::string name) {
	return script_classes_[name];
}

void ScriptSubSystem::createObject(unsigned int component_handle, std::string classname) {
	ScriptSystem *scriptsys = engine.getSystem<ScriptSystem>();
	auto class_s = scriptsys->getClass(classname);
	auto class_n = class_s->class_;
	auto obj = mono_object_new(scriptsys->getDomain(), class_n);
	components_[component_handle].scripts_.emplace_back(class_s, obj);

	if (class_s->script_method_ctor_) {
		MonoObject* exception = nullptr;
		mono_runtime_invoke(class_s->script_method_ctor_, obj, nullptr, &exception);

		if (exception) {
			std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
		}
	}
}	

void ScriptSubSystem::start() {
	for (auto &c : components_) {
		for (auto &s : c.scripts_) {
			if (s.script_class_->script_method_start_) {
				MonoObject* exception = nullptr;
				mono_runtime_invoke(s.script_class_->script_method_start_, s.script_object_, nullptr, &exception);

				if (exception) {
					std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
				}
			}
		}
	}
}

void ScriptSystem::update() {
	for (auto scene : engine.getScenes()) {
		for (auto space : scene->spaces_) {
			ScriptSubSystem *sub = (ScriptSubSystem *)space->getSubsystem(COMPONENT_SCRIPT);
			for (auto &c : sub->components_) {
				for (auto &s : c.scripts_) {
					if (s.script_class_->script_method_update_) {
						MonoObject* exception = nullptr;
						mono_runtime_invoke(s.script_class_->script_method_update_, s.script_object_, nullptr, &exception);

						if (exception) {
							std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
						}
					}
				}
			}
		}
	}
}

void ScriptSubSystem::cleanup() {
	for (auto &c : components_) {
		for (auto &s : c.scripts_) {
			if (s.script_class_->script_method_cleanup_) {
				MonoObject* exception = nullptr;
				mono_runtime_invoke(s.script_class_->script_method_cleanup_, s.script_object_, nullptr, &exception);

				if (exception) {
					std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
				}
			}
		}
	}
}

ScriptSubSystem::ScriptSubSystem(Space * space) : SubSystem(COMPONENT_SCRIPT, space) {
}

ComponentHandle ScriptSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

void ScriptSubSystem::initialize() {
	//for (auto &c : components_) {
		
	//}

	//engine.getSystem<ScriptSystem>()
	createObject(0, "Example");
	
	for (auto &c : components_) {
		for (auto &s : c.scripts_) {
			if (s.script_class_->script_method_init_) {
				MonoObject* exception = nullptr;
				mono_runtime_invoke(s.script_class_->script_method_init_, s.script_object_, nullptr, &exception);

				if (exception) {
					std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
				}
			}
		}
	}
}

ScriptComponent & ScriptSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * ScriptSubSystem::getBaseComponent(ComponentHandle handle) {
	return &components_[handle];
}

size_t ScriptSubSystem::getNumComponents() {
	return components_.size();
}

void ScriptSubSystem::removeComponent(ComponentHandle handle) {
	components_.erase(components_.begin() + handle);
}

ScriptSubSystem::~ScriptSubSystem() {
}

REFLECT_STRUCT_BEGIN(ScriptComponent, ScriptSystem, COMPONENT_SCRIPT)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
