#pragma once

#include <map>
#include <entt/entt.hpp>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <Common/HashedString.hpp>

#include "WorldContext.hpp"

namespace Grindstone {
	class WorldContextSet {
	public:
		WorldContextSet() : registry(), contexts() {}

		WorldContextSet(const WorldContextSet&) = delete;
		WorldContextSet& operator=(const WorldContextSet&) = delete;

		WorldContextSet(WorldContextSet&&) noexcept = default;
		WorldContextSet& operator=(WorldContextSet&&) noexcept = default;

		virtual ~WorldContextSet();

		[[nodiscard]] entt::registry& GetEntityRegistry() {
			return registry;
		}

		[[nodiscard]] Grindstone::WorldContext* GetContext(Grindstone::HashedString hashedString) {
			auto it = contexts.find(hashedString);
			if (it == contexts.end()) {
				return nullptr;
			}

			return it->second.Get();
		}

		[[nodiscard]] const entt::registry& GetEntityRegistry() const {
			return registry;
		}

		[[nodiscard]] const Grindstone::WorldContext* GetContext(Grindstone::HashedString hashedString) const {
			auto it = contexts.find(hashedString);
			if (it == contexts.end()) {
				return nullptr;
			}

			return it->second.Get();
		}

		void Create(Grindstone::HashedString hashedString, Grindstone::UniquePtr<Grindstone::WorldContext>&& cxt) {
			contexts.emplace(hashedString, std::move(cxt));
		}

		void Remove(Grindstone::HashedString hashedString) {
			contexts.erase(hashedString);
		}

	protected:
		entt::registry registry;
		std::map<Grindstone::HashedString, Grindstone::UniquePtr<Grindstone::WorldContext>> contexts;
	};
}
