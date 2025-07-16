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
		void* csharpObject = nullptr;

		REFLECT("CSharpScript")
	};

	void SetupCSharpScriptComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
	void DestroyCSharpScriptComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
}
