#pragma once

#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <HashedString.hpp>

class dtNavMesh;

namespace Grindstone::Ai {
	const Grindstone::ConstHashedString navMeshWorldContextName("Grindstone::Ai::NavMeshWorldContext");

	class NavMeshWorldContext : public Grindstone::WorldContext {
	public:
		NavMeshWorldContext() = default;
		NavMeshWorldContext(const NavMeshWorldContext&) = delete;
		NavMeshWorldContext(NavMeshWorldContext&&) noexcept = default;
		virtual ~NavMeshWorldContext() override = default;

		[[nodiscard]] static NavMeshWorldContext* GetActiveContext();
		static void SetActiveContext(NavMeshWorldContext& cxt);
		virtual void SetAsActive() override;
		dtNavMesh* GetNavMesh();

	protected:

		dtNavMesh* navMesh = nullptr;

	};
}
