#pragma once

#include <string>
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone::Scripting::CSharp {
	struct ScriptComponent {
		std::string assembly;
		std::string scriptNamespace;
		std::string scriptClassName;
		void* csharpObject = nullptr;

		static void Construct(Grindstone::WorldContextSet& cxtSet, entt::entity);
		static void Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity);

		REFLECT("CSharpScript")
	};
}
