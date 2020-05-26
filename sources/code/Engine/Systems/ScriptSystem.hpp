#ifndef _SCRIPT_SYSTEM_H
#define _SCRIPT_SYSTEM_H

#include "BaseSystem.hpp"

#include <string>
#include <vector>
#include <map>

#include <mono/jit/jit.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/assembly.h>

struct ScriptClass;

struct AssemblyPackage {
	MonoAssembly *assembly_;
	MonoImage *mono_image_;
	std::string path_;
	std::vector<ScriptClass *> classes_;
	AssemblyPackage(MonoAssembly *a, MonoImage *i, std::string p);
};

struct ScriptFields {
	enum class ScriptFieldType : uint8_t {
		SCRIPT_FIELD_UNKNOWN = 0,
		SCRIPT_FIELD_BOOL,
		SCRIPT_FIELD_STRING,
		SCRIPT_FIELD_UINT64,
		SCRIPT_FIELD_INT64,
		SCRIPT_FIELD_UINT32,
		SCRIPT_FIELD_INT32,
		SCRIPT_FIELD_UINT16,
		SCRIPT_FIELD_INT16,
		SCRIPT_FIELD_UINT8,
		SCRIPT_FIELD_INT8,
		SCRIPT_FIELD_DOUBLE,
		SCRIPT_FIELD_FLOAT,
		SCRIPT_FIELD_CHAR,
		SCRIPT_FIELD_OBJECT,
		SCRIPT_FIELD_DECIMAL,
		SCRIPT_FIELD_DATETIME,
	} type_;
	std::string name_;
	MonoClassField *field_;

	ScriptFields(ScriptFieldType t, std::string n, MonoClassField *f);
};

struct ScriptInstance {
	ScriptClass* script_class_;
	MonoObject* script_object_;
	ScriptInstance(ScriptClass* c, MonoObject *o);
};

class ScriptSystem;

struct ScriptComponent : public Component {
	ScriptComponent(GameObjectHandle object_handle, ComponentHandle id);

	std::vector<ScriptInstance> scripts_;
	REFLECT(COMPONENT_SCRIPT)
};

struct ScriptClass {
	std::string classname_;
	unsigned int assembly_id_;
	MonoClass* class_;
	MonoObject* script_object_;
	MonoMethod* script_method_ctor_;
	MonoMethod* script_method_init_;
	MonoMethod* script_method_start_;
	MonoMethod* script_method_update_;
	MonoMethod* script_method_cleanup_;
	std::vector<ScriptFields> fields_;
};

class ScriptSubSystem : public SubSystem {
	friend ScriptSystem;
public:
	ScriptSubSystem(Space *space);
	void start();
	void cleanup();
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual void initialize() override;
	ScriptComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
	void createObject(unsigned int component_handle, std::string classname);

	virtual ~ScriptSubSystem();
private:
	std::vector<ScriptComponent> components_;
};

class ScriptSystem : public System {
	friend ScriptSubSystem;
public:
	ScriptSystem();
	void update();
	inline ScriptClass *getClass(std::string);
	unsigned int loadAssembly(std::string path);
	void loadClass(unsigned int assembly_id, std::string classname);
	MonoDomain* getDomain();
	~ScriptSystem();
private:
	std::vector<AssemblyPackage *> assemblies_;
	std::map<std::string, ScriptClass *> script_classes_;
	MonoDomain *domain_;

	REFLECT_SYSTEM()
};

#endif