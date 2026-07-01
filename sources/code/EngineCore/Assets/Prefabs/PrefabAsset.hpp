#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <map>

#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	struct PrefabAsset : public Asset {
		struct ComponentDataDescriptor {

		};

		struct ComponentDescriptor {
			Grindstone::HashedString componentType;
			std::vector<ComponentDataDescriptor> fields;
		};

		struct EntityDescription {
			Grindstone::Uuid uuid;
			std::vector<ComponentDescriptor> components;
		};

		PrefabAsset(Grindstone::Uuid uuid) : Asset(uuid, uuid.ToString()) {}
		void InstantiateInWorldContext(Grindstone::WorldContextSet& worldContextSet, Grindstone::Uuid parentEntity = Grindstone::Uuid());

		std::vector<EntityDescription> entitiesDescriptions;
	};
}
