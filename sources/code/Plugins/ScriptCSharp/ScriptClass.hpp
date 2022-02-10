#pragma once

#include "mono/metadata/object-forward.h"

namespace Grindstone {
	namespace Scripting {
		namespace CSharp {
			struct ScriptClass {
				MonoClass* monoClass = nullptr;

				struct Methods {
					MonoMethod* constructor = nullptr;
					MonoMethod* onAttachComponent = nullptr;
					MonoMethod* onStart = nullptr;
					MonoMethod* onUpdate = nullptr;
					MonoMethod* onEditorUpdate = nullptr;
					MonoMethod* onDelete = nullptr;
				} methods;
			};
		}
	}
}
