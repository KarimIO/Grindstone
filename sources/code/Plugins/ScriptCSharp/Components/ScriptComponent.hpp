#pragma once

#include <string>
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Plugins/ScriptCSharp/ScriptClass.hpp"

namespace Grindstone::Scripting::CSharp {
	struct ScriptComponent {
		std::string assembly;
		std::string scriptNamespace;
		std::string scriptClass;
		ScriptClass* monoClass = nullptr;
		MonoObject* scriptObject = nullptr;

		REFLECT("CSharpScript")
	};

	void SetupCSharpScriptComponent(ECS::Entity& entity, void* componentPtr);
	void DestroyCSharpScriptComponent(ECS::Entity& entity);
}
