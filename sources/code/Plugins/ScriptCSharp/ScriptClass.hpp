#pragma once

#include <string>
#include <map>

#include <mono/metadata/class.h>
#include <mono/metadata/object-forward.h>

namespace Grindstone {
	namespace Scripting {
		namespace CSharp {
			struct ScriptField {
				// ScriptFieldType type;
				std::string name;
				MonoClassField* classFieldPtr;

				ScriptField() = default;
				ScriptField(std::string name, MonoClassField* classFieldPtr);
				virtual void Get(MonoObject* object, void* outValue);
				virtual void Set(MonoObject* object, void* outValue);
			};

			struct ScriptClass {
				std::string scriptNamespace;
				std::string scriptClassname;
				MonoClass* monoClass = nullptr;
				std::map<std::string, ScriptField> fields;

				struct Methods {
					MonoMethod* constructor = nullptr;
					MonoMethod* onAttachComponent = nullptr;
					MonoMethod* onStart = nullptr;
					MonoMethod* onUpdate = nullptr;
					MonoMethod* onEditorUpdate = nullptr;
					MonoMethod* onDelete = nullptr;
				} methods;

				ScriptClass() = default;
				ScriptClass(
					std::string scriptNamespace,
					std::string scriptClassname,
					MonoClass* monoClass
				);
			};
		}
	}
}
